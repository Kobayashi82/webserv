/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Cache.hpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/08/24 19:39:57 by vzurera-          #+#    #+#             */
/*   Updated: 2024/10/05 14:21:29 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <iostream>																						//	For standard input/output stream objects like std::cin, std::cout
#include <map>																							//	For std::map container
#include <list>																							//	For std::list container
#include <ctime>																						//	For time-related functions and types

#pragma region CacheInfo

	struct CacheInfo {

		//	Variables
		std::string	path;																				//	The file path or key associated with the cache entry
		std::string	content;																			//	The content stored in the cache for the given path
		size_t		size;																				//	The size of the cached content, typically in bytes

		std::string	mod_stime;																			//	The modification time of the file as string
		time_t		mod_time;																			//	The modification time of the file as seconds
		time_t		added_time;																			//	The time the file was added as seconds
		time_t		expire_time;																		//	The expiration time of the cache entry
		time_t		check_time;
		int			check_time_interval;																//	Time in seconds after which, if there's no access to the cache file, the modification time must be checked against the local file

		//	Constructors
		CacheInfo(const std::string & _path, const std::string & _content, std::string _mod_stime, time_t _mod_time, time_t _expire_time);	//	Parameterized constructor
		CacheInfo(const CacheInfo & src);																									//	Copy constructor

		//	Overloads
		CacheInfo & 	operator=(const CacheInfo & rhs);												//	Overload for assignation
		bool 			operator==(const CacheInfo & rhs) const;										//	Overload for comparison

		//	Methods
		bool			isExpired() const;																//	Return true if the CacheInfo has expired
		bool			check();																		//	Return true if the CacheInfo must be check for changes in disk (when 60 seconds older or more)

	};

#pragma endregion

#pragma region Cache

class Cache {

	public:

		//	Variables
		size_t														max_content_size;					//	Maximum size of the content allowed

		//	Constructors
		Cache();																						//	Default constructor
		Cache(int expire_time, size_t max_size, size_t max_content_size);								//	Parameterized constructor
		Cache(int expire_time, size_t max_size);														//	Parameterized constructor
		Cache(const Cache & src);																		//	Copy constructor
		~Cache();																						//	Destructor

		//	Overloads
		Cache &	operator=(const Cache & rhs);															//	Overload for assignation
		bool	operator==(const Cache & rhs) const;													//	Overload for comparison

		//	Methods
		bool		exists(const std::string & path);																				//	Checks if a file is in cache
		CacheInfo &	get(const std::string & path);																					//	Gets a pointer to the cache entry if it exists and updates the last usage order
		void		add(const std::string & path, const std::string & content, const std::string & mod_stime, time_t mod_time);		//	Adds a new entry. If the cache exceeds the maximum size, removes the least used entry
		void		remove(const std::string & path);																				//	Removes the cache entry associated with the given path
		void		remove_expired();																								//	Removes all expired cache entries
		void		remove_least_used();																							//	Removes the least recently used cache entry
		void		clear();																										//	Clears all cache entries

		void		add_caching(const std::string & path);
		bool		get_caching(const std::string & path);
		void		remove_caching(const std::string & path);
		void		cleanup_caching();

	private:
	
		//	Variables
		std::map <std::string, CacheInfo>							_cache;								//	Map to store cache entries, accessible by their path
		std::list <std::map <std::string, CacheInfo>::iterator>		_order;								//	List to maintain the order of cache entries based on usage
		size_t														_cache_size;						//	Current size of the cache in terms of the number of entries
		std::map <std::string, bool>								_caching;							//	Map storing cache entries that are currently being read before being added to the cache

		int															_expire_time;						//	Time in seconds after which a CacheInfo entry is considered expired
		size_t														_max_size;							//	Maximum number of files to keep in cache

};

#pragma endregion