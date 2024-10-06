/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Cache.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/08/24 19:40:07 by vzurera-          #+#    #+#             */
/*   Updated: 2024/10/06 13:35:28 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Cache.hpp"
#include "Utils.hpp"

#pragma region CacheInfo

	#pragma region Constructors

		CacheInfo::CacheInfo(const std::string & _path, const std::string & _content, std::string _mod_stime, time_t _mod_time, time_t _expire_time) :
			path(_path), content(_content), size(_content.size()), mod_stime(_mod_stime), mod_time(_mod_time), expire_time(time(NULL) + _expire_time) {
				added_time = time(NULL); check_time = time(NULL); check_time_interval = 5;
			}

		CacheInfo::CacheInfo(const CacheInfo & src) { *this = src; }

	#pragma endregion

	#pragma region Overloads

		CacheInfo & CacheInfo::operator=(const CacheInfo & rhs) {
			if (this != &rhs) { path = rhs.path; content = rhs.content; size = rhs.size; mod_stime = rhs.mod_stime; mod_time = rhs.mod_time; added_time = rhs.added_time; expire_time = rhs.expire_time; check_time = rhs.check_time; check_time_interval = rhs.check_time_interval; }
			return (*this);
		}

		bool CacheInfo::operator==(const CacheInfo & rhs) const {
			return (path == rhs.path && content == rhs.content && size == rhs.size && mod_stime == rhs.mod_stime && mod_time == rhs.mod_time && added_time == rhs.added_time && expire_time == rhs.expire_time && check_time == rhs.check_time && check_time_interval == rhs.check_time_interval);
		}

	#pragma endregion

	#pragma region isExpired

		bool CacheInfo::isExpired() const { return time(NULL) > expire_time; }

	#pragma endregion

	#pragma region Check

		bool CacheInfo::check() {
			if (time(NULL) > check_time + check_time_interval && Utils::file_modification_time_data(path) > mod_time) return (true);

			check_time = time(NULL);
			return (false);
		}

	#pragma endregion

#pragma endregion

#pragma region Cache

	#pragma region Constructors

		Cache::Cache() : max_content_size(1 * 1024 * 1024), _expire_time(600), _max_size(100) {}
		Cache::Cache(int expire_time, size_t max_size, size_t _max_content_size) : max_content_size(_max_content_size * 1024 * 1024), _expire_time(expire_time), _max_size(max_size) {}
		Cache::Cache(int expire_time, size_t max_size) : max_content_size(1 * 1024 * 1024), _expire_time(expire_time), _max_size(max_size) {}
		Cache::Cache(const Cache & src) { *this = src; }
		Cache::~Cache() { clear(); }

	#pragma endregion

	#pragma region Overloads

		Cache &	Cache::operator=(const Cache & rhs) {
			if (this != &rhs) {
				_cache = rhs._cache; _order = rhs._order; _cache_size = rhs._cache_size; _expire_time = rhs._expire_time; _max_size = rhs._max_size; max_content_size = rhs.max_content_size;
			} return (*this);
		}

		bool	Cache::operator==(const Cache & rhs) const {
			return (_cache == rhs._cache && _order == rhs._order && _cache_size == rhs._cache_size && _expire_time == rhs._expire_time && _max_size == rhs._max_size && max_content_size == rhs.max_content_size);
		}

	#pragma endregion

	#pragma region Cache

		#pragma region Exists

			bool Cache::exists(const std::string & path) {
				std::map <std::string, CacheInfo>::iterator it = _cache.find(path);
				if (it != _cache.end()) {
					CacheInfo & info = it->second;
					if (info.isExpired() || info.check()) {
						_order.remove(it);
						_cache.erase(it);
						--_cache_size;

						return (false);
					}

					_order.remove(it);
					_order.push_front(it);

					return (true);
				}

				return (false);
			}

		#pragma endregion

		#pragma region Get

			CacheInfo & Cache::get(const std::string & path) {
				std::map <std::string, CacheInfo>::iterator it = _cache.find(path);
				if (it != _cache.end()) {
					CacheInfo & info = it->second;
					if (info.isExpired() || info.check()) {
						_order.remove(it);
						_cache.erase(it);
						--_cache_size;

						throw std::runtime_error("Expired or Check");
					}

					_order.remove(it);
					_order.push_front(it);

					return (info);
				}

				throw std::runtime_error("Not in cache");
			}

		#pragma endregion

		#pragma region Add

			void Cache::add(const std::string & path, const std::string & content, const std::string & mod_stime, time_t mod_time) {
				if (path.empty() || content.empty() || content.size() > max_content_size) return;

				std::map <std::string, CacheInfo>::iterator it = _cache.find(path);

				if (it != _cache.end()) {
					_order.remove(it);
					_cache.erase(it);
					--_cache_size;
				} else if (_cache_size >= _max_size) {
					remove_expired();
					if (_cache_size >= _max_size) remove_least_used();
				}

				std::pair <std::map <std::string, CacheInfo>::iterator, bool> new_it;

				new_it = _cache.insert(std::make_pair(path, CacheInfo(path, content, mod_stime, mod_time, _expire_time)));
				_order.push_front(new_it.first);
				++_cache_size;
			}

		#pragma endregion

		#pragma region Remove

			#pragma region Remove

				void Cache::remove(const std::string & path) {
					std::map<std::string, CacheInfo>::iterator it = _cache.find(path);
					if (it != _cache.end()) {
						_order.remove(it);
						_cache.erase(it);
						--_cache_size;
					}
				}

			#pragma endregion

			#pragma region Remove Expired

				void Cache::remove_expired() {
					std::map<std::string, CacheInfo>::iterator it = _cache.begin();

					while (it != _cache.end()) {
						if (it->second.isExpired()) {
							std::map<std::string, CacheInfo>::iterator current = it++;
							_order.remove(current);
							_cache.erase(current);
							--_cache_size;
						} else ++it;
					}
				}

			#pragma endregion

			#pragma region Remove Least Used

				void Cache::remove_least_used() {
					if (_cache.empty()) return;

					std::map <std::string, CacheInfo>::iterator lru_iter = *(_order.rbegin());

					--_cache_size;
					_cache.erase(lru_iter);

					_order.pop_back();
				}

			#pragma endregion

		#pragma endregion

		#pragma region Clear

			void Cache::clear() {
				_order.clear(); _cache.clear();
			}

		#pragma endregion

	#pragma endregion

#pragma region Caching

	void Cache::add_caching(const std::string & path) {
		_caching[path] = "true";
	}

	bool Cache::get_caching(const std::string & path) {
		return (_caching[path]);
	}

	void Cache::remove_caching(const std::string & path) {
		_caching.erase(path);
	}

	void Cache::cleanup_caching() {
 		for (std::map<std::string, bool>::iterator it = _caching.begin(); it != _caching.end(); ) {
			if (it->second == false) {
				std::map<std::string, bool>::iterator curr_it = it++;
				_caching.erase(curr_it);
			} else ++it;
		}
	}

#pragma endregion

#pragma endregion
