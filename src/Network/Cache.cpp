/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Cache.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: vzurera- <vzurera-@student.42malaga.com    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/08/24 19:40:07 by vzurera-          #+#    #+#             */
/*   Updated: 2024/08/26 19:29:40 by vzurera-         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Cache.hpp"

#pragma region CacheInfo

	#pragma region Constructors

		Cache::CacheInfo::CacheInfo(const std::string & _path, const std::string & _content, time_t _expire) : path(_path), content(_content), expire(time(NULL) + _expire) {}
		Cache::CacheInfo::CacheInfo(const CacheInfo & src) { *this = src; }

	#pragma endregion

	#pragma region Overloads

		Cache::CacheInfo & Cache::CacheInfo::operator=(const CacheInfo & rhs) {
			if (this != &rhs) { path = rhs.path; content = rhs.content; size = rhs.size; expire = rhs.expire; }
			return (*this);
		}

		bool Cache::CacheInfo::operator==(const CacheInfo & rhs) const {
			return (path == rhs.path && content == rhs.content && size == rhs.size && expire == rhs.expire);
		}

	#pragma endregion

	#pragma region isExpired

		bool Cache::CacheInfo::isExpired() const { return time(NULL) > expire; }

	#pragma endregion

#pragma endregion

#pragma region Cache

	#pragma region Constructors

		Cache::Cache() : _expire_time(600), _max_size(100), _max_content_size(1 * 1024 * 1024) {}
		Cache::Cache(int expire_time, size_t max_size, size_t max_content_size) : _expire_time(expire_time), _max_size(max_size), _max_content_size(max_content_size * 1024 * 1024) {}
		Cache::Cache(int expire_time, size_t max_size) : _expire_time(expire_time), _max_size(max_size), _max_content_size(1 * 1024 * 1024) {}
		Cache::Cache(const Cache & src) { *this = src; }
		Cache::~Cache() { clear(); }

	#pragma endregion

	#pragma region Overloads

		Cache &	Cache::operator=(const Cache & rhs) {
			if (this != &rhs) {
				_cache = rhs._cache; _order = rhs._order; _cache_size = rhs._cache_size; _expire_time = rhs._expire_time; _max_size = rhs._max_size; _max_content_size = rhs._max_content_size;
			} return (*this);
		}

		bool	Cache::operator==(const Cache & rhs) const {
			return (_cache == rhs._cache && _order == rhs._order && _cache_size == rhs._cache_size && _expire_time == rhs._expire_time && _max_size == rhs._max_size && _max_content_size == rhs._max_content_size);
		}

	#pragma endregion

	#pragma region Cache

		#pragma region Get

			Cache::CacheInfo * Cache::get(const std::string & path) {
				std::map <std::string, CacheInfo>::iterator it = _cache.find(path);
				if (it != _cache.end()) {
					CacheInfo & info = it->second;
					if (info.isExpired()) {
						_order.remove(it);
						_cache.erase(it);
						--_cache_size;

						return (NULL);
					}

					_order.remove(it);
					_order.push_front(it);

					return (&info);
				}

				return (NULL);
			}

		#pragma endregion

		#pragma region Add

			void Cache::add(const std::string & path, const std::string & content) {
				if (path.empty() || content.empty() || content.size() > _max_content_size) return;
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

				new_it = _cache.insert(std::make_pair(path, CacheInfo(path, content, _expire_time)));
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

#pragma endregion
