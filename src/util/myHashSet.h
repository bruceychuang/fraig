/****************************************************************************
  FileName     [ myHashSet.h ]
  PackageName  [ util ]
  Synopsis     [ Define HashSet ADT ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2014-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#ifndef MY_HASH_SET_H
#define MY_HASH_SET_H

#include <vector>

using namespace std;

//---------------------
// Define HashSet class
//---------------------
// To use HashSet ADT,
// the class "Data" should at least overload the "()" and "==" operators.
//
// "operator ()" is to generate the hash key (size_t)
// that will be % by _numBuckets to get the bucket number.
// ==> See "bucketNum()"
//
// "operator ==" is to check whether there has already been
// an equivalent "Data" object in the HashSet.
// Note that HashSet does not allow equivalent nodes to be inserted
//

template <class Data>
class HashSet
{ 
public:
   HashSet(size_t b = 0) : _numBuckets(0), _buckets(0) { if (b != 0) init(b); }
   ~HashSet() { reset(); }

   // TODO: implement the HashSet<Data>::iterator
   // o An iterator should be able to go through all the valid Data
   //   in the Hash
   // o Functions to be implemented:
   //   - constructor(s), destructor
   //   - operator '*': return the HashNode
   //   - ++/--iterator, iterator++/--
   //   - operators '=', '==', !="
   //
   class iterator
   {
      friend class HashSet<Data>;
   public:
      iterator(){}
      iterator(const iterator& i) : _size(i._size), _sizeL(i._sizeL), _col(i._col), _vec(i._vec){}
      ~iterator(){}
      Data& operator*(){ return (*_vec)[_col]; }
      iterator& operator++(){
         if(++_col >= _vec->size()){
            _col = 0;
            ++_vec;
            --_sizeL;
            while(_sizeL>0 && _vec->empty()){
               if(_sizeL > 1) ++_vec;
               --_sizeL;
            }
         }
         return *this;
      }
      iterator operator++(int){
         iterator it(*this);
         ++(*this);
         return it;
      }
      iterator& operator--(){
         if(_col > 1) --_col;
         else if(_sizeL == 0) ++_sizeL;
         else if(_sizeL != _size){
            while(_sizeL < _size && _vec->empty()){
               --_vec;
               ++_sizeL;
            }
            if(_vec->empty) _col = 0;
            else _col = _vec->size()-1;
         }
         return *this;
      }
      iterator operator--(int){
         iterator it(*this);
         --(*this);
         return it;
      }
      size_t level(){ return _sizeL; }
      size_t levelSize(){ return (*_vec).size()w; }
      void operator=(const iterator& i){
         _size = i._size;
         _sizeL = i._sizeL;
         _col = i._col;
         _vec = _vec;
      }
      bool operator==(const iterator& i){ return (_sizeL == i._sizeL && _col == i._col); }
      bool operator!=(const iterator& i){ return !(*this == i); }
   private:
      size_t _size, _sizeL, _col;
      vector<Data>* _vec;
   };

   void init(size_t b) { _numBuckets = b; _buckets = new vector<Data>[b]; }
   void reset() {
      _numBuckets = 0;
      if (_buckets) { delete [] _buckets; _buckets = 0; }
   }
   void clear() {
      for (size_t i = 0; i < _numBuckets; ++i) _buckets[i].clear();
   }
   size_t numBuckets() const { return _numBuckets; }

   vector<Data>& operator [] (size_t i) { return _buckets[i]; }
   const vector<Data>& operator [](size_t i) const { return _buckets[i]; }

   // TODO: implement these functions
   //
   // Point to the first valid data
   iterator begin() const{
      iterator it;
      it._size = it._sizeL = _numBuckets;
      it._col = 0;
      it._vec = _buckets;
      while(it._sizeL > 0 && it._vec->empty()){
         if(it._sizeL > 1) ++it._vec;
         --it._sizeL;
      }
      return it;
   }
   // Pass the end
   iㄑterator end() const{
      iterator it;
      it._size = _numBuckets;
      it._sizeL = 0;
      it._col = 0;
      it._vec = _buckets+_numBuckets-1;
      return it;
   }
   // return true if no valid data
   bool empty() const{
      int i=_numBuckets;
      while(i>0){
         --i;
         if(!(_buckets+i)->empty()) break;
      }
      return !i;
   }
   // number of valid data
   size_t size() const{
      size_t s = 0;
      iterator it = begin();
      while(it != end()){
         ++s;
         ++it;
      }
      return s;
   }

   // check if d is in the hash...
   // if yes, return true;
   // else return false;
   bool check(const Data& d) const{
      vector<Data>* vec = _buckets+bucketNum(d);
      for(size_t i=0; i<vec->size(); ++i){
         if((*vec)[i] == d) return true;
      }
      return false;
   }

   // query if d is in the hash...
   // if yes, replace d with the data in the hash and return true;
   // else return false;
   bool query(Data& d) const{
      vector<Data>* vec = _buckets+bucketNum(d);
      for(size_t i=0; i<vec->size(); ++i){
         if((*vec)[i] == d){
            d = (*vec)[i];
            return true;
         }
      }
      return false;
   }

   // update the entry in hash that is equal to d (i.e. == return true)
   // if found, update that entry with d and return true;
   // else insert d into hash as a new entry and return false;
   bool update(const Data& d){
      vector<Data>* vec = _buckets+bucketNum(d);
      for(size_t i=0; i<vec->size(); ++i){
         if((*vec)[i] == d){
            d = (*vec)[i];
            return true;
         }
      }
      insert(d);
      return false;
   }

   // return true if inserted successfully (i.e. d is not in the hash)
   // return false is d is already in the hash ==> will not insert
   bool insert(const Data& d){
      if(check(d)) return false;
      (_buckets+bucketNum(d))->push_back(d);
      return true;
   }

   // return true if removed successfully (i.e. d is in the hash)
   // return fasle otherwise (i.e. nothing is removed)
   bool remove(const Data& d){
      vector<Data>* vec = _buckets+bucketNum(d);
      for(size_t i=0; i<vec->size(); ++i){
         if((*vec)[i] == d){
            vec->erase(vec->begin()+i);
            return true;
         }
      }
      return false;
   }

private:
   // Do not add any extra data member
   size_t            _numBuckets;
   vector<Data>*     _buckets;

   size_t bucketNum(const Data& d) const {
      return (d() % _numBuckets); }
};

#endif // MY_HASH_SET_H
