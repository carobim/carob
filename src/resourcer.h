/******************************
** Tsunagari Tile Engine     **
** resourcer.h               **
** Copyright 2011 OmegaSDG   **
******************************/

#ifndef RESOURCER_H
#define RESOURCER_H

#include <map>
#include <string>
#include <utility>

#include <Gosu/Gosu.hpp>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <zip.h>

// Time to live for empty cache members. Defaults to 5 minutes.
#define CACHE_EMPTY_TTL "300"

class GameWindow;

//! Resourcer Class
/*!
	This class provides the engine's global resource handling and caching.
*/
class Resourcer
{
public:
	//! Resourcer Constructor
	Resourcer(GameWindow* window, const std::string& filename);

	//! Resourcer Destructor
	~Resourcer();

	//! Resourcer Initializer
	bool init();

	//! Requests an image resource from cache.
	Gosu::Image* getImage(const std::string& name);

	//! Requests a bitmap that can be used to construct subimages from cache.
	Gosu::Bitmap getBitmap(const std::string& name);

	//! Converts a subrectangle of a Bitmap into an Image.
	Gosu::Image* bitmapSection(const Gosu::Bitmap& src,
	        unsigned x, unsigned y, unsigned w, unsigned h, bool tileable);

	//! Requests a string resource from cache.
	std::string getString(const std::string& name);

	//! Requests an XML resource from cache.
	xmlDoc* getXMLDoc(const std::string& name);
	
	//! Close resource request and drop from cache. IMPORTANT!!!
	void drop(const std::string& name);

	//! Returns a music stream from disk or cache.
	Gosu::Sample* getSample(const std::string& name);

private:
	//! Read a resource from disk into memory. Returns NULL on error.
	Gosu::Buffer* read(const std::string& name);

	//! Helper function 
	std::string path(const std::string& entry_name) const;

	GameWindow* window;
	zip* z;
	const std::string zip_filename;
	
	//! Resource Cache
	/*!
		The key is the cached file's name, and the value is a pair of a
		"tally" and a pointer to the data. The "tally" is increased each
		time something requests the resource, and decreased each time
		something is finished with that resource. When the tally reaches
		zero, nothing is using the resource, and it is dropped after a
		few minutes. The cache drop timer is in a thread.
	*/
	std::map<std::string, std::pair<int, void*> > cache;
	
	//! Modify the cache tally. Logs a Developer warning on tally underrun.
	void cacheTally(const std::string& key, const int mod);
};

#endif

