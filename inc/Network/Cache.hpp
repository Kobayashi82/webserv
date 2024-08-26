/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Cache.hpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/08/24 19:39:57 by vzurera-          #+#    #+#             */
/*   Updated: 2024/08/26 22:06:18 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include <iostream>																						//	For standard input/output stream objects like std::cin, std::cout
#include <map>																							//	For std::map container
#include <list>																							//	For std::list container
#include <ctime>																						//	For time-related functions and types

class Cache {

	public:

		#pragma region CacheInfo

		struct CacheInfo {

			//	Variables
			std::string	path;																			//	The file path or key associated with the cache entry
			std::string	content;																		//	The content stored in the cache for the given path
			size_t		size;																			//	The size of the cached content, typically in bytes
			time_t		expire;																			//	The expiration time of the cache entry

			//	Constructors
			CacheInfo(const std::string & _path, const std::string & _content, time_t _expire);			//	Parameterized constructor
			CacheInfo(const CacheInfo & src);															//	Copy constructor

			//	Overloads
			CacheInfo & 	operator=(const CacheInfo & rhs);											//	Overload for asignation
			bool 			operator==(const CacheInfo & rhs) const;									//	Overload for comparison

			//	Methods
			bool			isExpired() const;															//	Return true if the CacheInfo expired

		};

		#pragma endregion

		//	Constructors
		Cache();																						//	Default constructor
		Cache(int expire_time, size_t max_size, size_t max_content_size);								//	Parameterized constructor
		Cache(int expire_time, size_t max_size);														//	Parameterized constructor
		Cache(const Cache & src);																		//	Copy constructor
		~Cache();																						//	Destructor

		//	Overloads
		Cache &	operator=(const Cache & rhs);															//	Overload for asignation
		bool	operator==(const Cache & rhs) const;													//	Overload for comparison

		//	Methods
		CacheInfo *	get(const std::string & path);														//	Get a pointer to cache entry if it exists and update the last usage order
		void		add(const std::string & path, const std::string & content);							//	Add a new entry. If the cache exceeds the maximum size, removes the least used
		void		remove(const std::string & path);													//	Removes the cache entry associated with the given path
		void		remove_expired();																	//	Removes all expired cache entries
		void		remove_least_used();																//	Removes the least recently used cache entry
		void		clear();																			//	Clears all cache entries

	private:
	
		//	Variables
		std::map <std::string, CacheInfo>							_cache;								//	Map to store cache entries, accessible by their path
		std::list <std::map <std::string, CacheInfo>::iterator>		_order;								//	List to maintain the order of cache entries based on usage
		size_t														_cache_size;						//	Current size of the cache in terms of the number of entries

		int															_expire_time;						//	Time in seconds after which a CacheInfo entry is considered expired
		size_t														_max_size;							//	Maximum number of files to keep in cache
		size_t														_max_content_size;					//	Maximum size of the content allowed

};
