/*
author: suninf
description: 基于三元状态树( Ternary Search Trees )实现的tst_map，
             结合二叉树的空间效率和digital tries的时间效率，是非常
			 有效的基于字符串作为key的关联容器。
*/

#include <functional>
#include <string>
#include <utility>
#include <iterator>
using std::iterator_traits;

namespace tst {

template< typename T, typename Ch >
struct tnode 
{
	typedef tnode* node_ptr;
	tnode( Ch ch ) : 
	splitchar(ch), lokid(0), hikid(0), eqkid(0), pdata(0) {}
	
	Ch splitchar;
	node_ptr lokid, hikid, eqkid;
	T* pdata;
};

template<typename T, typename Ch = char, typename Comp = std::less<Ch> >
class tst_map
{
public:
	typedef std::basic_string<Ch, std::char_traits<Ch>, std::allocator<Ch> > tstring;
	typedef tnode<T,Ch>* node_ptr;
	typedef tstring	key_type;
	
	typedef T value_type;
	typedef T& reference;
	typedef T* pointer;
	typedef T const& const_reference;
	typedef T const* const_pointer;

public:
	tst_map() 
		: root_(0), comp_(Comp()), size_(0) {}

	tst_map( const tst_map& st ) // default copy ctor
		: root_(0), comp_(Comp()), size_(0)
	{
		st.foreach( __insert_helper<tst_map>(*this) );
	}
	
	template<typename U, typename Compare>
	tst_map( const tst_map<U, Ch, Compare>& st )
		: root_(0), comp_(Comp()), size_(0)
	{
		st.foreach( __insert_helper<tst_map>(*this) );
	}

	template<typename Iter>
	tst_map( Iter beg, Iter end )
		: root_(0), comp_(Comp()), size_(0)
	{
		while ( beg != end )
		{
			insert( beg->first, beg->second );
			++beg;
		}
	}

	tst_map& operator = ( const tst_map& m ) // default assignment
	{
		if ( (void*)this != (void*)(&m) )
		{
			clear();
			m.foreach( __insert_helper<tst_map>(*this) );
		}
		return *this;
	}

	template<typename U, typename Compare>
	tst_map& operator = ( const tst_map<U, Ch, Compare>& m )
	{
		if ( (void*)this != (void*)(&m) )
		{
			clear();
			m.foreach( __insert_helper<tst_map>(*this) );
		}
		return *this;
	}

	~tst_map()
	{
		__destroy( root_ );
	}

	pointer insert( const tstring& str, const T& val )// may be just update if exist
	{
		pointer pos = 0;
		root_ = __insert( root_, str.c_str(), val, pos );
		return pos;
	}

	template<typename Iter>
	void insert( Iter beg, Iter end ) // insert [beg, end), value_type: pair<string, T>
	{
		while ( beg != end )
		{
			insert( beg->first, beg->second );
			++beg;
		}
	}

	pointer insert( const std::pair<tstring, T>& pair_val )
	{
		pointer pos = 0;
		root_ = __insert( root_, pair_val.first.c_str(), pair_val.second, pos );
		return pos;
	}

	pointer find( const tstring& str )// exist if not return 0
	{
		node_ptr p = root_;
		const Ch* s = str.c_str();
		while ( p )
		{
			if ( comp_( *s, p->splitchar ) )
				p = p->lokid;
			else if ( !comp_(*s, p->splitchar) && !comp_( p->splitchar, *s ) )
			{
				if ( *(++s) == 0 )
				{
					return p->pdata; // if p->pdata 0，means not exist
				}
				p = p->eqkid;
			} 
			else
				p = p->hikid;
		}
		return 0;
	}

	reference operator[]( const tstring& str )
	{
		pointer pos = 0;
		root_ = __insert( root_, str.c_str(), pos );
		return *pos;
	}

	size_t size() { return size_; };

	bool empty() { return size_==0; }

	template< typename Seq >
	void nearsearch( const tstring& str, int d, Seq& c ) // near-neighbor
	{
		tstring strtmp;
		c.clear();
		__near_search<Seq>( root_, str.c_str(), d, strtmp, c );
	}

	template< typename Seq >
	void pmsearch( const tstring& str, Seq& c ) // partial-match
	{
		tstring strtmp;
		c.clear();
		__pmsearch( root_, str.c_str(), strtmp, c );
	}

	template<typename Func>
	void foreach( Func f )
	{
		tstring str;
		__travel( root_, str, f );
	}


	template< typename Seq > // for vector,deque,list, value_type: pair<string, T>
	void sequence( Seq& c )
	{
		tstring str;
		c.clear();
		__travel( root_, str, __push_back<Seq>(c) );
	}

	void swap( tst_map& m )
	{
		std::swap( root_, m.root_ );
		std::swap( size_, m.size_ );
	}

	bool remove( const tstring& str )
	{
		node_ptr p = root_;
		const Ch* s = str.c_str();
		while ( p )
		{
			if ( comp_( *s, p->splitchar ) )
				p = p->lokid;
			else if ( !comp_(*s, p->splitchar) && !comp_( p->splitchar, *s ) )
			{
				if ( *(++s) == 0 && p->pdata )
				{
					delete p->pdata;
					--size_;
					p->pdata = 0;
					return true;
				}
				p = p->eqkid;
			} 
			else
				p = p->hikid;
		}
		return false;
	}

	bool erase( const tstring& str ) { return remove(str); }

	void clear()
	{
		__destroy( root_ );
	}

	// const versions
	const_pointer find( const tstring& str ) const// exist if not return 0
	{
		node_ptr p = root_;
		const Ch* s = str.c_str();
		while ( p )
		{
			if ( comp_( *s, p->splitchar ) )
				p = p->lokid;
			else if ( !comp_(*s, p->splitchar) && !comp_( p->splitchar, *s ) )
			{
				if ( *(++s) == 0 )
				{
					return p->pdata;
				}
				p = p->eqkid;
			} 
			else
				p = p->hikid;
		}
		return 0;
	}

	template< typename Seq >
	void pmsearch( const tstring& str, Seq& c ) const
	{
		tstring strtmp;
		c.clear();
		__pmsearch( root_, str.c_str(), strtmp, c );
	}

	template< typename Seq > 
	void sequence( Seq& c ) const
	{
		tstring str;
		c.clear();
		__travel( root_, str, __push_back<Seq>(c) );
	}
	template< typename Seq >
	void nearsearch( const tstring& str, int d, Seq& c ) const
	{
		tstring strtmp;
		c.clear();
		__near_search<Seq>( root_, str.c_str(), d, strtmp, c );
	}

	template<typename Func>
	void foreach( Func f ) const
	{
		tstring str;
		__travel( root_, str, f );
	}

	const T operator[]( const tstring& str ) const
	{
		const T* pos = find(str);
		return pos ? *pos : T();
	}

	size_t size() const { return size_; };

	bool empty() const { return size_==0; }

private: // inner use for implement
	template< typename Seq >
	void __pmsearch( node_ptr p, const Ch* s, tstring& cur_str, Seq& c )
	{
		if ( *s == 0 )
			return;

		if ( !p )
			return;

		if ( *s=='.' || comp_(*s, p->splitchar) )
		{
			__pmsearch( p->lokid, s, cur_str, c );
		}
		if ( *s=='.' || ( !comp_(*s, p->splitchar) && !comp_( p->splitchar, *s ) ) )
		{// match a single character
			cur_str.push_back( p->splitchar );
			if ( *(s+1) == 0 && p->pdata )
			{
				c.push_back( std::make_pair( cur_str, *(p->pdata) ) );
			}
			else if ( *(s+1) )
			{
				__pmsearch( p->eqkid, s+1, cur_str, c );
			}
			cur_str.erase( cur_str.begin() + cur_str.size() - 1 );
		}
		if ( *s=='.' || comp_( p->splitchar, *s ) )
		{
			__pmsearch( p->hikid, s, cur_str, c );
		}
	}

	template< typename Seq >
	void __near_search( node_ptr p, const Ch* s, int d, tstring& cur_str, Seq& seq ) // at most d different characters
	{
		if ( p==0 || d<0 )
			return;
		if ( d>0 || comp_(*s, p->splitchar) )
		{
			__near_search( p->lokid, s, d, cur_str, seq );
		}

		cur_str.push_back( p->splitchar );
		__near_search( p->eqkid, *s?s+1:s, 
			( !comp_(*s, p->splitchar) && !comp_( p->splitchar, *s ) ) ? d : d-1,
			cur_str, seq );
		cur_str.erase( cur_str.begin() + cur_str.size() - 1 );

		if ( d>0 || comp_( p->splitchar, *s ) )
		{
			__near_search( p->hikid, s, d, cur_str, seq );
		}

		if ( p->pdata ) // find a data
		{
			// move to s+1; if equals，d remain, else d-1
			if ( __strlen(*s?s+1:s) <= 
				(( !comp_(*s, p->splitchar) && !comp_( p->splitchar, *s ) ) ? d : d-1) )
			{
				seq.push_back( std::make_pair( cur_str+p->splitchar, *(p->pdata) ) );
			}
		}
	}

	// travel throuth the sequence, already sorted by comp
	template< typename Func >
	void __travel( node_ptr p, tstring& cur_str, Func& f )
	{
		if ( !p )
			return;
		__travel( p->lokid, cur_str, f );

		cur_str.push_back( p->splitchar );
		__travel( p->eqkid, cur_str, f );
		cur_str.erase( cur_str.begin() + cur_str.size()-1 );

		__travel( p->hikid, cur_str, f );

		if ( p->pdata )// travel child first, then self if exist
		{
			f( cur_str+p->splitchar, *(p->pdata) );
		}
	}

	void __destroy(node_ptr& p)
	{
		if ( p==0 )
			return;
		__destroy( p->lokid );
		__destroy( p->eqkid );
		__destroy( p->hikid );
		if ( p->pdata )
		{
			delete p->pdata;
			p->pdata = 0;
			--size_;
		}
		delete p;
		p = 0;
	}

	node_ptr __insert( node_ptr p, const Ch* s, const T& t, pointer& pos ) // for insert
	{
		if ( *s == 0 )// ignore empty string
			return p;

		// not empty string
		if ( p==0 )
			p = new tnode<T,Ch>( *s );

		if ( comp_( *s, p->splitchar )  )
			p->lokid = __insert( p->lokid, s, t, pos );
		else if ( !comp_(*s, p->splitchar) && !comp_( p->splitchar, *s ) )
		{
			if ( *(s+1) == 0 )// arrive end, save data
			{
				if ( 0 != p->pdata ) // data already exist, just update
				{
					*(p->pdata) = t;
				}
				else
				{
					++size_;
					p->pdata = new T( t );
				}
				pos = p->pdata;
			}
			else
			{
				p->eqkid = __insert( p->eqkid, ++s, t, pos );
			}
		}
		else
			p->hikid = __insert( p->hikid, s, t, pos );
		return p;
	}

	node_ptr __insert( node_ptr p, const Ch* s, pointer& pos ) // for operator[]
	{
		if ( *s == 0 )// ignore empty string
			return p;

		if ( p==0 )
			p = new tnode<T,Ch>( *s );

		if ( comp_( *s, p->splitchar )  )
			p->lokid = __insert( p->lokid, s, pos );
		else if ( !comp_(*s, p->splitchar) && !comp_( p->splitchar, *s ) )
		{
			if ( *(s+1) == 0 )// save data: pdata available
			{
				if ( p->pdata == 0 )
				{
					++size_;
					p->pdata = new T();
				}
				pos = p->pdata;
			}
			else
			{
				p->eqkid = __insert( p->eqkid, ++s, pos );
			}
		}
		else
			p->hikid = __insert( p->hikid, s, pos );
		return p;
	}

	// const versions
	template< typename Seq >
	void __pmsearch( node_ptr p, const Ch* s, tstring& cur_str, Seq& c ) const
	{
		if ( *s == 0 )
			return;

		if ( !p )
			return;

		if ( *s=='.' || comp_(*s, p->splitchar) )
		{
			__pmsearch( p->lokid, s, cur_str, c );
		}
		if ( *s=='.' || ( !comp_(*s, p->splitchar) && !comp_( p->splitchar, *s ) ) )
		{// match a single character
			cur_str.push_back( p->splitchar );
			if ( *(s+1) == 0 && p->pdata )
			{
				c.push_back( std::make_pair( cur_str, *(p->pdata) ) );
			}
			else if ( *(s+1) )
			{
				__pmsearch( p->eqkid, s+1, cur_str, c );
			}
			cur_str.erase( cur_str.begin() + cur_str.size() - 1 );
		}
		if ( *s=='.' || comp_( p->splitchar, *s ) )
		{
			__pmsearch( p->hikid, s, cur_str, c );
		}
	}

	template< typename Seq >
	void __near_search( node_ptr p, const Ch* s, int d, tstring& cur_str, Seq& seq ) const
	{
		if ( p==0 || d<0 )
			return;
		if ( d>0 || comp_(*s, p->splitchar) )
		{
			__near_search( p->lokid, s, d, cur_str, seq );
		}

		cur_str.push_back( p->splitchar );
		__near_search( p->eqkid, *s?s+1:s, 
			( !comp_(*s, p->splitchar) && !comp_( p->splitchar, *s ) ) ? d : d-1,
			cur_str, seq );
		cur_str.erase( cur_str.begin() + cur_str.size() - 1 );

		if ( d>0 || comp_( p->splitchar, *s ) )
		{
			__near_search( p->hikid, s, d, cur_str, seq );
		}

		if ( p->pdata ) // find a data
		{
			// move to s+1; if equals，d remain, else d-1
			if ( __strlen(*s?s+1:s) <= 
				(( !comp_(*s, p->splitchar) && !comp_( p->splitchar, *s ) ) ? d : d-1) )
			{
				seq.push_back( std::make_pair( cur_str+p->splitchar, *(p->pdata) ) );
			}
		}
	}

	template< typename Func >
	void __travel( node_ptr p, tstring& cur_str, Func& f ) const
	{
		if ( !p )
			return;
		__travel( p->lokid, cur_str, f );

		cur_str.push_back( p->splitchar );
		__travel( p->eqkid, cur_str, f );
		cur_str.erase( cur_str.begin() + cur_str.size()-1 );

		__travel( p->hikid, cur_str, f );

		if ( p->pdata )// travel child first, then self if exist
		{
			f( cur_str+p->splitchar, *(p->pdata) );
		}
	}

	template< typename Seq >
	struct __push_back
	{
		Seq& seq_;
		__push_back( Seq& s ) : seq_(s) {}
		void operator()( const tstring& str, const T& t )
		{
			seq_.push_back( std::make_pair( str, t ) );
		}
	};

	template< typename TSTMap >
	struct __insert_helper 
	{
		TSTMap& m_;
		__insert_helper( TSTMap& m ) : m_(m) {}
		template<typename U>
		void operator()( const tstring& str, const U& t )
		{
			m_.insert( str, t );
		}
	};

	static int __strlen( const Ch* s )
	{
		int len = 0;
		while ( *s++ )
		{
			++len;
		}
		return len;
	}
private:
	Comp comp_;
	node_ptr root_;
	size_t size_;

};

template<typename T, typename Ch, typename Comp>
void swap( tst_map<T, Ch, Comp>& lhs, tst_map<T, Ch, Comp>& rhs )
{
	lhs.swap( rhs );
}

} // namespace tst

