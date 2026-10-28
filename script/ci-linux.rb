#!/usr/bin/env ruby
# Run from the checkout root: ruby script/ci-linux.rb [job ...]
# Requires Docker; supports this repository's checkout/run-only Linux workflow.
require 'digest'
require 'fileutils'
require 'json'
require 'open3'
require 'securerandom'
require 'shellwords'
require 'yaml'

class LinuxCI
  def initialize(root)
    @root = root
    @config = YAML.safe_load(File.read(File.join(root, '.circleci/config.yml')))
  end

  def run(selected)
    jobs = @config.fetch('workflows').reject { |name, _| name == 'version' }
                  .values.flat_map { |workflow| workflow.fetch('jobs') }.uniq
    raise 'Only plain workflow job names are supported' unless jobs.all? { |job| job.is_a?(String) }
    unknown = selected - jobs
    raise "Unknown jobs: #{unknown.join(', ')}" unless unknown.empty?
    jobs = selected unless selected.empty?
    jobs.each { |name| validate(name) }
    prepared = jobs.map { |name| prepare_job(name) }.compact
    prepared.each { |job| run_job(job) }
  end

  private

  def validate(name)
    job = @config.fetch('jobs').fetch(name)
    unsupported = job.keys - %w[docker steps shell environment working_directory]
    raise "#{name}: unsupported options: #{unsupported.join(', ')}" unless unsupported.empty?
    images = job.fetch('docker')
    unless images.size == 1 && (images.first.keys - %w[image environment]).empty?
      raise "#{name}: expected one Docker image, without executor options"
    end
    images.first.fetch('image')
    job.fetch('steps').each do |step|
      next if step == 'checkout'
      unless step.is_a?(Hash) && step.keys == ['run']
        raise "#{name}: unsupported step: #{step.inspect}"
      end
      run = step.fetch('run')
      next if run.is_a?(String)
      unless run.is_a?(Hash) && (run.keys - %w[command name shell environment working_directory]).empty?
        raise "#{name}: unsupported run options: #{run.inspect}"
      end
      run.fetch('command')
    end
  end

  def docker(*args)
    raise "Docker command failed: #{['docker', *args].shelljoin}" unless system('docker', *args)
  end

  def inspect_image(reference)
    output, _error, status = Open3.capture3('docker', 'image', 'inspect', reference)
    status.success? ? JSON.parse(output).fetch(0) : nil
  end

  def cache_name(reference)
    # Digest references cannot be tags; preserve their identity in a legal tag.
    reference = reference.sub('@sha256:', ':sha256-')
    reference += ':latest' unless reference.split('/').last.include?(':')
    "carob-ci/#{reference}"
  end

  def environment(values)
    (values || {}).flat_map { |key, value| ['--env', "#{key}=#{value}"] }
  end

  def pull_image(name, reference)
    path = File.join(Dir.home, '.cache/carob-ci/state.json')
    FileUtils.mkdir_p(File.dirname(path))
    # Serialize the read/pull/write cycle so concurrent runners share the cooldown.
    File.open("#{path}.lock", File::RDWR | File::CREAT, 0o600) do |lock|
      lock.flock(File::LOCK_EX)
      begin
        state = JSON.parse(File.read(path))
        raise JSON::ParserError, 'expected an object' unless state.is_a?(Hash)
      rescue Errno::ENOENT
        state = {}
      rescue JSON::ParserError => e
        warn "Ignoring invalid pull cache: #{e.message}"
        state = {}
      end
      entry = state[reference]
      last_pull = entry.is_a?(Hash) ? entry['timestamp'] : entry
      age = last_pull.is_a?(Numeric) ? Time.now.to_f - last_pull : nil
      if age && age >= 0 && age <= 12 * 60 * 60
        if entry.is_a?(Hash) && entry['result'] == 'unavailable'
          puts "Skipping #{name}: image unavailable for this platform (cached within 12 hours)"
          return false
        end
        raise entry.fetch('error') if entry.is_a?(Hash) && entry['result'] == 'error'
        if inspect_image(reference)
          puts "--> Using local #{reference} (pulled within 12 hours)"
          return true
        end
      end
      output = +''
      status = Open3.popen2e('docker', 'pull', reference) do |stdin, stream, wait|
        stdin.close
        stream.each_line do |line|
          print line
          output << line
        end
        wait.value
      end
      unavailable = !status.success? && output.match?(/no matching manifest for \S+|no match for platform in manifest/i)
      error = "Docker command failed: #{['docker', 'pull', reference].shelljoin}"
      state[reference] = {
        'timestamp' => Time.now.to_f,
        'result' => status.success? ? 'available' : (unavailable ? 'unavailable' : 'error')
      }
      state[reference]['error'] = error unless status.success? || unavailable
      temporary = "#{path}.#{Process.pid}-#{SecureRandom.hex(6)}"
      begin
        File.write(temporary, JSON.pretty_generate(state) + "\n", mode: 'w', perm: 0o600)
        File.rename(temporary, path)
      ensure
        FileUtils.rm_f(temporary)
      end
      if unavailable
        puts "Skipping #{name}: image unavailable for this platform"
        return false
      end
      raise error unless status.success?
      true
    end
  end

  def prepare_job(name)
    job = @config.fetch('jobs').fetch(name)
    image = job.fetch('docker').first
    # Use an absolute path: Docker does not expand CircleCI's default ~/project.
    workdir = job.fetch('working_directory', '/root/project').sub(/\A~(?=\/|$)/, '/root')
    puts "==> #{name} (#{image.fetch('image')})"
    return unless pull_image(name, image.fetch('image'))
    base = inspect_image(image.fetch('image'))
    host_json, status = Open3.capture2('docker', 'info', '--format', '{{json .}}')
    raise 'Cannot determine Docker host architecture' unless status.success?
    host = JSON.parse(host_json)
    aliases = { 'aarch64' => 'arm64', 'x86_64' => 'amd64', 'armv7l' => 'arm', 'armv6l' => 'arm', 'i386' => '386', 'i686' => '386' }
    host_arch = aliases.fetch(host.fetch('Architecture'), host.fetch('Architecture'))
    unless base.fetch('Architecture') == host_arch && base.fetch('Os') == host.fetch('OSType')
      puts "Skipping #{name}: image is #{base.fetch('Os')}/#{base.fetch('Architecture')}, Docker host is #{host.fetch('OSType')}/#{host_arch}"
      return
    end
    steps = job.fetch('steps')
    checkout = steps.index('checkout')
    setup = checkout ? steps.take(checkout) : []
    cached_name = cache_name(image.fetch('image'))
    cache_key = Digest::SHA256.hexdigest(JSON.generate([
      1, base.fetch('Id'), setup,
      job['shell'], workdir, image['environment'], job['environment']
    ]))
    cached = inspect_image(cached_name)
    reuse = cached && cached.dig('Config', 'Labels', 'carob-ci.setup-key') == cache_key
    puts "--> Reusing setup image #{cached_name}" if reuse
    prepared = { name: name, workdir: workdir, steps: checkout ? steps.drop(checkout) : steps }
    if reuse
      return prepared.merge(image: cached.fetch('Id'))
    end
    with_container(name, image.fetch('image'), workdir) do |container|
      run_steps(name, container, workdir, setup)
      docker('commit', '--change', "LABEL carob-ci.setup-key=#{cache_key}", container, cached_name)
      # Retain IDs: another job may overwrite the same stable cache tag.
      prepared.merge(image: inspect_image(cached_name).fetch('Id'))
    end
  end

  def run_job(prepared)
    name = prepared.fetch(:name)
    puts "==> Running #{name}"
    with_container(name, prepared.fetch(:image), prepared.fetch(:workdir)) do |container|
      run_steps(name, container, prepared.fetch(:workdir), prepared.fetch(:steps))
    end
  end

  def with_container(name, reference, workdir)
    job = @config.fetch('jobs').fetch(name)
    image = job.fetch('docker').first
    container = "ci-linux-#{Process.pid}-#{SecureRandom.hex(6)}"
    begin
      # An open stdin keeps sh alive without depending on utilities in old images.
      docker('create', '--name', container, '-i', '--entrypoint', '/bin/sh',
             *environment(image['environment']), *environment(job['environment']),
             reference, '-s')
      started = system('docker', 'start', container)
      state_json, inspected = Open3.capture2('docker', 'inspect', '--format', '{{json .State}}', container)
      raise "Cannot inspect container #{container}" unless inspected.success?
      state = JSON.parse(state_json)
      unless started && state.fetch('Running')
        logs, = Open3.capture2e('docker', 'logs', container)
        details = [state['Error'], logs].compact.join("\n")
        if details.match?(/exec format error/i)
          puts "Skipping #{name}: image cannot execute on this platform"
          return
        end
        raise "Container failed to start for #{name}: #{details}"
      end
      docker('exec', container, '/bin/sh', '-ec', 'mkdir -p "$1"', 'sh', workdir)
      yield container
    ensure
      system('docker', 'rm', '--force', container)
    end
  end

  def run_steps(name, container, workdir, steps)
    job = @config.fetch('jobs').fetch(name)
    steps.each do |step|
      if step == 'checkout'
        # docker cp also works with remote daemons; a bind mount would not.
        docker('cp', File.join(@root, '.'), "#{container}:#{workdir}")
        next
      end
      run = step.fetch('run')
      run = { 'command' => run } if run.is_a?(String)
      puts "--> #{run.fetch('name', run.fetch('command'))}"
      shell = run['shell'] || job['shell']
      command = if shell
                  [*Shellwords.split(shell), '-ec', run.fetch('command')]
                else
                  # Minimal images may not ship Bash; use it with pipefail when available.
                  ['/bin/sh', '-ec', 'if command -v bash >/dev/null 2>&1; then exec bash -eo pipefail -c "$1"; else exec /bin/sh -ec "$1"; fi',
                   'sh', run.fetch('command')]
                end
      docker('exec', '--workdir', run.fetch('working_directory', workdir),
             *environment(run['environment']), container, *command)
    end
  end
end

if $PROGRAM_NAME == __FILE__
  $stdout.sync = true
  begin
    LinuxCI.new(Dir.pwd).run(ARGV)
  rescue StandardError => e
    warn e.message
    exit 1
  rescue Interrupt
    warn 'Interrupted'
    exit 130
  end
end
