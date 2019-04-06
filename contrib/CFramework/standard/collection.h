/*
* Copyright (C) 2017-2018 Tauri JayD <https://www.>
*
* This program is free software; you can redistribute it and/or modify it
* under the terms of the GNU General Public License as published by the
* Free Software Foundation; either version 2 of the License, or (at your
* option) any later version.
*
* This program is distributed in the hope that it will be useful, but WITHOUT
* ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
* FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
* more details.
*
* You should have received a copy of the GNU General Public License along
* with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef __Collection_H__
#define __Collection_H__

#include <standard/misc.h>
#include <standard/events.h>

namespace System
{
    namespace Collection
    {
        template<typename T, std::size_t Size>
        inline std::size_t GetArrLength(T(&)[Size]) { return Size; }

        template<typename T>
        inline bool array_size_check(unsigned int id, T& array)
        {
            unsigned int size = GetArrLength(array);
            return id < size;
        }

        inline int enum_count() { return 0; }
        template<typename ENUM, typename... Args>
        constexpr int enum_count(ENUM a, Args... args) { return 1 + enum_member_count(args...); }


        ///
        /// Use only std::map unorderedmap std::set unorderedset (can't use multi version)
        ///

        template<class TMAP>
        inline bool GetContains(TMAP& mp, typename TMAP::key_type ky, typename TMAP::iterator& result)
        {
            result = mp.find(ky);
            return result != mp.end();
        }

        template<class TMAP>
        constexpr inline bool GetContains(TMAP& mp, typename TMAP::key_type ky)
        {
            return mp.find(ky) != mp.end();
        }

        ///
        /// Use only std::map unorderedmap std::set unorderedset (can't use multi version)
        ///

        template<class TMAP, typename VALUE>
        inline bool GetValue(TMAP& mp, typename TMAP::key_type ky, VALUE& result)
        {
            typename TMAP::iterator itr = mp.find(ky);
            if (itr != mp.end())
            {
                result = itr->second;
                return true;
            }
            return false;
        }

        template <typename K, typename T>
        class TypeContainer
        {
        public:
            typedef std::map<K, T*> StorageType;

            TypeContainer() : ReadLock(nullptr), WriteLock(nullptr), UnLock(nullptr) {}
            ~TypeContainer() { Clear(); }

            void SetLockMethods(void(*_ReadLock)(), void(*_WriteLock)(), void(*_UnLock)())
            {
                ReadLock = _ReadLock;
                WriteLock = _WriteLock;
                UnLock = _UnLock;
            }

            T* Create(K key)
            {
                T* t = new T;
                Add(key, t);
                return t;
            }

            void Add(K key, T* type)
            {
                if (Exist(key))
                    return;

                if (WriteLock)
                    WriteLock();

                m_container[key] = type;

                if (UnLock)
                    UnLock();
            }

            void Remove(K key)
            {
                if (WriteLock)
                    WriteLock();

                StorageType::iterator i;
                if (GetContains(m_container, key, i))
                {
                    delete i->second;
                    m_container.erase(i);
                }

                if (UnLock)
                    UnLock();
            }

            T* Get(K key)
            {
                return (*this)[key];
            }

            T* operator[](K key)
            {
                T* ret = nullptr;
                StorageType::iterator i;

                if (ReadLock)
                    ReadLock();

                if (GetContains(m_container, key, i))
                    ret = i->second;

                if (UnLock)
                    UnLock();
                return nullptr;
            }


            bool Exist(K key)
            {
                StorageType::iterator i;
                if (ReadLock)
                    ReadLock();

                bool ret = (GetContains(m_container, key, i));

                if (UnLock)
                    UnLock();
                return ret;
            }

            void Clear(void)
            {
                if (WriteLock)
                    WriteLock();

                while (!m_container.empty())
                    Remove(m_container.begin()->first);

                if (UnLock)
                    UnLock();
            }

        private:
            TypeContainer(TypeContainer const&);

            StorageType m_container;

            void(*ReadLock)();
            void(*WriteLock)();
            void(*UnLock)();
        };

        template<typename T>
        class StringIndexedMap
        {
        public:
            typedef std::pair<std::string, T>   NamePair;
            typedef std::vector<NamePair>       NameList;

            NameList m_map;

            void clear()
            {
                m_map.clear();
            }

            void remove(unsigned int pos)
            {
                if (pos < m_map.size())
                {
                    std::swap(m_map[pos], m_map.back());
                    m_map.pop_back();
                }
            }

            void remove(typename NameList::iterator __itr)
            {
                if (__itr != m_map.end())
                {
                    std::swap(*__itr, m_map.back());
                    m_map.pop_back();
                }
            }

            void remove(typename NameList::const_iterator __itr)
            {
                if (__itr != m_map.end())
                {
                    std::swap(*__itr, m_map.back());
                    m_map.pop_back();
                }
            }

            void erase(typename NameList::iterator __itr)
            {
                if (__itr != m_map.end())
                {
                    std::swap(*__itr, m_map.back());
                    m_map.pop_back();
                }
            }

            void erase(typename NameList::const_iterator __itr)
            {
                if (__itr != m_map.end())
                {
                    std::swap(*__itr, m_map.back());
                    m_map.pop_back();
                }
            }

            static bool stringCompare(const NamePair &left, const NamePair &right)
            {
                return strcmp(left.first.c_str(), right.first.c_str()) <= 0;
                //for( std::string::const_iterator lit = left.first.begin(), rit = right.first.begin(); lit != left.first.end() && rit != right.first.end(); ++lit, ++rit )
                //{
                //   if ( *lit == *rit )
                //       continue;
                //    return(*lit < *rit ? true : false);
                //}
                //
                //return(left.first.size() < right.first.size() ? true : false);
            }

            void push_back(NamePair value) { m_map.push_back(value); }
            void Sort() { std::sort(m_map.begin(), m_map.end(), &StringIndexedMap::stringCompare); }

            /*
            * searches for a value in sorted array
            *   arr is an array to search in
            *   value is searched value
            *   left is an index of left boundary
            *   right is an index of right boundary
            * returns position of searched value, if it presents in the array
            * or -1, if it is absent
            */
            int binarySearch(const char* value, int left, int right)
            {
                int size = right;
                while (left <= right)
                {
                    int middle = (left + right) / 2;
                    if (size <= middle)
                        break;
                    const char* middleChar = m_map[middle].first.c_str();
                    int diff = strcmp(middleChar, value);
                    if (!diff)
                        return middle;
                    else if (diff > 0)
                        right = middle - 1;
                    else
                        left = middle + 1;
                }
                return -1;
            }

            bool Find(NamePair* result, const char* pattern, unsigned int* ElementPos = NULL)
            {
                if (m_map.empty())
                    return false;

                int cpos = binarySearch(pattern, 0, m_map.size());
                if (cpos != -1)
                {
                    if (ElementPos)
                        *ElementPos = cpos;

                    if (result)
                        *result = m_map[cpos];
                    return true;
                }
                return false;
            }
        };
    }
};

#endif //__Collection_H__