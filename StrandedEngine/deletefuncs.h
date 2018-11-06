#ifndef __DELETE_FUNCS_H__
#define __DELETE_FUNCS_H__

#include <map>
#include <list>
#include <vector>
#include <deque>
#include <algorithm>
#include <functional>

/*
 * \brief ....
 * @brief: Templated safe-delete function.
 * @param: T *& ptr - A reference to a pointer of type T
 * @return:
 */
template <class T> inline void Delete(T& ptr)
{
	// Check for NULL-pointer
	if (ptr != nullptr)
	{
		// Delete object pointed to.
		delete ptr;
		ptr = nullptr;
	}
}

/*
 * @brief: Templated safe-release comm function.
 * @param: T *& ptr - A reference to a pointer of type T
 * @return:
 */
template <class T> inline void Release(T& ptr)
{
	// Check for NULL-pointer
	if (ptr != NULL)
	{
		// Release comm object.
		ptr->Release();
		ptr = NULL;
	}
}

/*
 * @brief: Templated safe-delete array function.
 * @param: T *& ptr - A reference to an array pointer of type T
 * @return:
 */
template <class T> inline void DeleteArray(T& ptr)
{
	// Check for NULL-pointer
	if (ptr != nullptr)
	{
		// Delete object pointed to.
		delete[] ptr;
		ptr = nullptr;
	}
}

/*
 * @brief: Templated safe-delete function for vectors of pointers.
 * @param: std::vector<T>& obj - A reference to a vector of objects of type T.
 * @return:
 */
template <class T> inline void DeleteVectorPointers(std::vector<T>& obj)
{
	// Loop through the vector and delete all objects pointed to.
	for_each(obj.begin(), obj.end(), Delete<T>);

	// Clear out the vector.
	obj.clear();
}

/*
* @brief: Templated safe-delete function for vectors of pointers.
* @param: std::vector<T>& obj - A reference to a vector of objects of type T.
* @return:
*/
template <class T> inline void DeleteVectorPointersArrays(std::vector<T>& obj)
{
	// Loop through the vector and delete all objects pointed to.
	for_each(obj.begin(), obj.end(), DeleteArray<T>);

	// Clear out the vector.
	obj.clear();
}

/*
* @brief: Templated safe-delete function for a deque of pointers.
* @param: std::deque<T>& obj - A reference to a deque of objects of type T.
* @return:
*/
template <class T> inline void DeleteDequePointers(std::deque<T>& obj)
{
	// Loop through the deque and delete all objects pointed to.
	for_each(obj.begin(), obj.end(), Delete<T>);

	// Clear out the deque.
	obj.clear();
}

/*
* @brief: Templated safe-delete function for lists of pointers.
* @param: std::list<T>& obj - A reference to a list of objects of type T.
* @return:
*/
template <class T> inline void DeleteListPointers(std::list<T>& obj)
{
	// Loop through the list and delete all objects pointed to.
	for_each(obj.begin(), obj.end(), Delete<T>);

	// Clear out the list.
	obj.clear();
}

/*
 * @brief: Templated safe-delete function for maps where the 'second' is a pointer to an allocated object.
 * @param: std::map<key, ptr>& obj - A reference to a map where the 'second' is an object of type T.
 * @return:
 */
template <class key, class ptr> inline void DeleteMapPointers(std::map<key, ptr>& obj)
{
	// Loop through the map and delete all objects pointed to.
	for (typename std::map<key, ptr>::iterator iter = obj.begin(); iter != obj.end(); ++iter)
	{
		// Delete object pointed to by the dereferenced iterator..
		Delete(iter->second);
	}

	// Clear out the map.
	obj.clear();
}

/*
* @brief: Templated safe-delete function for maps where the 'second' is a pointer to an allocated object.
* @param: std::map<key, ptr>& obj - A reference to a map where the 'second' is an object of type T.
* @return:
*/
template <class key, class ptr> inline void DeleteMapDirectXPointers(std::map<key, ptr>& obj)
{
	// Loop through the map and delete all objects pointed to.
	for (typename std::map<key, ptr>::iterator iter = obj.begin(); iter != obj.end(); ++iter)
	{
		// Have directX release the data
		Release(iter->second);
	}

	// Clear out the map.
	obj.clear();
}

#endif
