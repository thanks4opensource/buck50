// buck50: Test and measurement firmware for “Blue Pill” STM32F103 development board
// Copyright (C) 2019,2020 Mark R. Rubin aka "thanks4opensource"
//
// This file is part of buck50.
//
// The buck50 program is free software: you can redistribute it
// and/or modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation, either version 3 of
// the License, or (at your option) any later version.
//
// The buck50 program is distributed in the hope that it will be
// useful, but WITHOUT ANY WARRANTY; without even the implied warranty
// of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// (LICENSE.txt) along with the buck50 program.  If not, see
// <https://www.gnu.org/licenses/gpl.html>


#ifndef regbits_hxx
#define regbits_hxx

#define REGBITS_MAJOR_VERSION   1
#define REGBITS_MINOR_VERSION   0
#define REGBITS_MICRO_VERSION   1


namespace regbits {

// forward refs
template<typename MSKD_WORD, typename MSKD_CLSS> class Mskd;
template<typename COPY_WORD, typename COPY_CLSS> class Copy ;
template<typename REG_WORD,  typename  REG_CLSS> class Reg ;



template<typename WORD, typename CLSS> class Pos {
  public:
    // friends
    template <typename BITS_WORD, typename BITS_CLSS> friend class Bits;
    template <typename MSKD_WORD, typename MSKD_CLSS> friend class Mskd;
    template <typename SHFT_WORD, typename SHFT_CLSS> friend class Shft;
    template <typename COPY_WORD, typename COPY_CLSS> friend class Copy;
    template <typename  REG_WORD, typename  REG_CLSS> friend class Reg ;


    // constructors
    //
    explicit
    constexpr
    Pos()
    :   _pos(static_cast<WORD>(0))
    {}

    explicit
    constexpr
    Pos(
    const WORD  pos)
    :   _pos(pos)
    {}

#ifdef REGBITS_COPY_CTOR  // implementing this impacts object passing performance
    constexpr
    Pos(
    const Pos<WORD, CLSS>   &other)
    :   _pos(other._pos)
    {}
#else
    Pos(const Pos<WORD, CLSS> &other) = default;
#endif


    // for passing constexpr instance by value without requiring storage
    Pos<WORD, CLSS> operator+()
    const
    {
        return *this;  // don't need: return Pos(_pos);
    }


    // resettter
    Pos<WORD, CLSS> &operator()(
    const WORD  pos)
    {
        _pos = pos;
        return *this;
    }


    // assignments
    //
    Pos<WORD, CLSS> operator=(
    const Pos<WORD, CLSS>   &other)
    {
        _pos = other._pos;
        return *this;
    }

    Pos<WORD, CLSS> operator=(
    const WORD  word)
    {
        _pos = word;
        return *this;
    }


    // accessor
    constexpr WORD pos() const { return _pos; }


    // comparisons
    //
    constexpr
    bool operator==(
    const Pos<WORD, CLSS>    other)
    const
    {
        return other._pos == _pos;
    }

    constexpr
    bool operator!=(
    const Pos<WORD, CLSS>    other)
    const
    {
        return  other._pos != _pos;
    }


  private:
    // do not implement this, even as private
    // operator WORD() {}


  protected:
    WORD    _pos;

};  // template<typename WORD, typename CLSS> class Pos



template<typename WORD, typename CLSS> class Bits {
  public:
    // friends
    template <typename MSKD_WORD, typename MSKD_CLSS> friend class Mskd;
    template <typename COPY_WORD, typename COPY_CLSS> friend class Copy;
    template <typename REG_WORD,  typename  REG_CLSS> friend class Reg ;


    // constructors
    //
    explicit
    constexpr
    Bits()
    :   _bits(static_cast<WORD>(0))
    {}

    explicit
    constexpr
    Bits(
    const WORD  bits)
    :   _bits(bits)
    {}

    constexpr
    Bits(
    const     WORD          bits,
    const Pos<WORD, CLSS>   pos )
    :   _bits(bits << pos._pos)
    {}

    Bits(
    const Mskd<WORD, CLSS>      mskd)
    :   _bits(mskd._bits)
    {}

    Bits(
    const Reg<WORD, CLSS>   &reg)
    :   _bits(reg._word)
    {}

    Bits(
    volatile const Reg<WORD, CLSS>  &reg)
    :   _bits(reg._word)
    {}

#ifdef REGBITS_COPY_CTOR  // implementing this impacts object passing performance
    constexpr
    Bits(
    const Bits<WORD, CLSS>  &other)
    :   _bits(other._bits)
    {}
#else
    Bits(const Bits<WORD, CLSS> &other) = default;
#endif

    // for passing constexpr instance by value without requiring storage
    Bits<WORD, CLSS> operator+()
    const
    {
        return *this;  // don't need: return Bits(_bits)
    }


    // resettter
    Bits<WORD, CLSS> &operator()(
    const WORD  bits)
    {
        _bits = bits;
        return *this;
    }


    // assignments
    //
    Bits<WORD, CLSS> operator=(
    const Bits<WORD, CLSS>  &other)
    {
        _bits = other._bits;
        return *this;
    }

    Bits<WORD, CLSS> operator=(
    const WORD  bits)
    {
        _bits = bits;
        return *this;
    }

    Bits<WORD, CLSS> operator=(
    const Mskd<WORD, CLSS>  mskd)
    {
        _bits = mskd._bits;
        return *this;
    }

    Bits<WORD, CLSS> operator=(
    const Reg<WORD, CLSS>   &reg)
    {
        _bits = reg._word;
        return *this;
    }

    Bits<WORD, CLSS> operator=(
    volatile const Reg<WORD, CLSS>  &reg)
    {
        _bits = reg._word;
        return *this;
    }

    // accessor
    constexpr WORD bits() const { return _bits; }


    // bitwise operators
    //
    constexpr
    Bits<WORD, CLSS> operator|(
    const Bits<WORD, CLSS>   other)
    const
    {
        return Bits<WORD, CLSS>(_bits | other._bits);
    }

    constexpr
    Bits<WORD, CLSS> operator|(
    const Mskd<WORD, CLSS>   &mskd)
    const
    {
        return Bits<WORD, CLSS>(_bits | mskd._bits);
    }

    Bits<WORD, CLSS> operator|=(
    const Bits<WORD, CLSS>   other)
    {
        _bits |= other._bits;
        return *this;
    }

    constexpr
    Bits<WORD, CLSS> operator|=(
    const Mskd<WORD, CLSS>   &mskd)
    {
        _bits |= mskd._bits;
        return *this;
    }

    constexpr
    Bits<WORD, CLSS> operator-(
    const Bits<WORD, CLSS>   other)
    const
    {
        return Bits<WORD, CLSS>(_bits & ~other._bits);
    }

    constexpr
    Bits<WORD, CLSS> operator-(
    const Mskd<WORD, CLSS>   &mskd)
    const
    {
        return Bits<WORD, CLSS>(_bits &~ mskd._bits);
    }

    Bits<WORD, CLSS> operator-=(
    const Bits<WORD, CLSS>   other)
    {
        _bits = bits & ~other._bits;
        return *this;
    }

    constexpr
    Bits<WORD, CLSS> operator-=(
    const Mskd<WORD, CLSS>   &mskd)
    {
        _bits = bits & ~mskd._bits;
        return *this;
    }


    // comparisons
    //
    constexpr
    bool operator==(
    const Bits<WORD, CLSS>   other)
    const
    {
        return other._bits == _bits;
    }

    constexpr
    bool operator!=(
    const Bits<WORD, CLSS>   other)
    const
    {
        return  other._bits != _bits;
    }


  private:
    // do not implement this, even as private
    // operator WORD() {}


  protected:
    WORD    _bits;

};  // template<typename WORD, typename CLSS> class Bits



template<typename WORD, typename CLSS> class Mskd {
  public:
    // friends
    template <typename BITS_WORD, typename BITS_CLSS> friend class Bits;
    template <typename COPY_WORD, typename COPY_CLSS> friend class Copy;
    template <typename  REG_WORD, typename  REG_CLSS> friend class Reg ;


    // constructors
    //
    explicit
    constexpr
    Mskd()
    :   _mask(static_cast<WORD>(0)),
        _bits(static_cast<WORD>(0))
    {}

    constexpr
    Mskd(
    const     WORD          mask,
    const     WORD          bits,
    const Pos<WORD,CLSS>    pos )
    :   _mask(mask << pos._pos),
        _bits(bits << pos._pos)
    {}

    constexpr
    Mskd(
    const WORD  mask,
    const WORD  bits)
    :   _mask(mask),
        _bits(bits)
    {}

    constexpr
    Mskd(
    const Bits<WORD, CLSS>  bits)
    :   _mask(bits._bits),
        _bits(bits._bits)
    {}

#ifdef REGBITS_COPY_CTOR  // implementing this impacts object passing performance
    constexpr
    Mskd(
    const Mskd<WORD, CLSS>  &other)
    :   _mask(other._mask),
        _bits(other._bits)
    {}
#else
    Mskd(const Mskd<WORD, CLSS> &other) = default;
#endif

    // for passing constexpr instance by value without requiring storage
    Mskd<WORD, CLSS> operator+()
    const
    {
        return Mskd(_mask, _bits);   // can't have: return *this
    }


    // resettter
    Mskd<WORD, CLSS> &operator()(
    const WORD  mask,
    const WORD  bits)
    {
        _mask = mask;
        _bits = bits;
        return *this;
    }


    // assignments
    //
    Mskd<WORD, CLSS> &operator=(
    const Mskd<WORD, CLSS>  &other)
    {
        _mask = other._mask;
        _bits = other._bits;
        return *this;
    }

    Mskd<WORD, CLSS> &operator=(
    const Bits<WORD, CLSS>  &bits)
    {
        _mask = bits._bits;
        _bits = bits._bits;
        return *this;
    }

    Mskd<WORD, CLSS> &operator=(
    const Reg<WORD, CLSS>   &reg)
    {
        _bits = reg._word;
        // do not modify _mask
        return *this;
    }

    Mskd<WORD, CLSS> &operator=(
    volatile const Reg<WORD, CLSS>  &reg)
    {
        _bits = reg._word;
        // do not modify _mask
        return *this;
    }


    // accessors
    constexpr WORD mask() const { return _mask; }
    constexpr WORD bits() const { return _bits; }


    // bitwise operators
    //
    constexpr
    Mskd<WORD, CLSS> operator|(
    const Mskd<WORD, CLSS>  other)
    const
    {
        return Mskd<WORD, CLSS>(_mask | other._mask, _bits | other._bits);
    }

    constexpr
    Mskd<WORD, CLSS> operator|(
    const Bits<WORD, CLSS>   bits)
    const
    {
        return Mskd<WORD, CLSS>(_mask | bits._bits, _bits | bits._bits);
    }

    Mskd<WORD, CLSS> operator|=(
    const Mskd<WORD, CLSS>  other)
    {
        _mask |= other._mask;
        _bits |= other._bits;
        return *this;
    }

    Mskd<WORD, CLSS> operator|=(
    const Bits<WORD, CLSS>   bits)
    {
        _mask |= bits._bits;
        _bits |= bits._bits;
        return *this;
    }

    constexpr
    Mskd<WORD, CLSS> operator-(
    const Mskd<WORD, CLSS>  other)
    const
    {
        return Mskd<WORD, CLSS>(_mask & ~other._mask, _bits & ~other._bits);
    }

    constexpr
    Mskd<WORD, CLSS> operator-(
    const Bits<WORD, CLSS>   bits)
    const
    {
        return Mskd<WORD, CLSS>(_mask & ~bits._bits, _bits & ~bits._bits);
    }

    Mskd<WORD, CLSS> operator-=(
    const Mskd<WORD, CLSS>  other)
    {
        _mask = _mask & ~other._mask;
        _bits = bits & ~other._bits;
        return *this;
    }

    Mskd<WORD, CLSS> operator-=(
    const Bits<WORD, CLSS>   bits)
    {
        _mask = _mask & ~bits._bits;
        _bits = _mask & ~bits._bits;
        return *this;
    }


    // comparisons
    //
    constexpr
    bool operator==(
    const Mskd<WORD, CLSS>  other)
    const
    {
        return other._mask == _mask && other._bits == _bits;
    }

    constexpr
    bool operator!=(
    const Mskd<WORD, CLSS>  other)
    const
    {
        return  other._mask != _mask || other._bits != _bits;
    }

    constexpr
    bool operator<(
    const Mskd<WORD, CLSS>  other)
    const
    {
        return _bits < other._bits;
    }

    constexpr
    bool operator<=(
    const Mskd<WORD, CLSS>  other)
    const
    {
        return _bits <= other._bits;
    }

    constexpr
    bool operator>(
    const Mskd<WORD, CLSS>  other)
    const
    {
        return _bits > other._bits;
    }

    constexpr
    bool operator>=(
    const Mskd<WORD, CLSS>  other)
    const
    {
        return _bits >= other._bits;
    }


  private:
    // do not implement this, even as private
    // operator WORD() {}


  protected:
    WORD    _mask,
            _bits;

};  // template<typename WORD, typename CLSS> class Mskd



template<typename WORD, typename CLSS> class Shft {
  public:
    // friends
    template <typename COPY_WORD, typename COPY_CLSS> friend class Copy;
    template <typename  REG_WORD, typename  REG_CLSS> friend class Reg ;

    // constructors
    //
    explicit
    constexpr
    Shft()
    :   _mask(static_cast<WORD>(0)),
        _pos (static_cast<WORD>(0))
    {}

    constexpr
    Shft(
    const     WORD          mask,
    const Pos<WORD,CLSS>    pos )
    :   _mask(mask << pos._pos),
        _pos(pos              )
    {}

#ifdef REGBITS_COPY_CTOR  // implementing this impacts object passing performance
    constexpr
    Shft(
    const Shft<WORD, CLSS>  &other)
    :   _mask(other._mask),
        _pos (other._pos )
    {}
#else
    Shft(const Shft<WORD, CLSS> &other) = default;
#endif

    // for passing constexpr instance by value without requiring storage
    constexpr
    Shft<WORD, CLSS> operator+()
    const
    {
        return *this;
    }


    // resettter
    Shft<WORD, CLSS> &operator()(
    const WORD  mask,
    const WORD  pos )
    {
        _mask = mask;
        _pos  = pos ;
        return *this;
    }


    // assignments
    //
    Shft<WORD, CLSS> &operator=(
    const Shft<WORD, CLSS>  &other)
    {
        _mask = other._mask;
        _pos  = other._pos ;
        return *this;
    }


    // accessors
    uint32_t        mask() const { return _mask; }
    Pos<WORD, CLSS> pos () const { return _pos ; }


  protected:
    uint32_t            _mask;
    Pos<WORD, CLSS>     _pos;

};  // template<typename WORD, typename CLSS> class Shft



template<typename WORD,  typename CLSS> class Reg {
  public:
    // friends
    template <typename BITS_WORD, typename BITS_CLSS> friend class Bits;
    template <typename MSKD_WORD, typename MSKD_CLSS> friend class Mskd;
    template <typename COPY_WORD, typename COPY_CLSS> friend class Copy;


    // constructors
    //
    Reg<WORD, CLSS>() {}

    explicit
    constexpr
    Reg<WORD, CLSS>(
    const WORD  word)
    :   _word(word)
    {}

    constexpr
    Reg<WORD, CLSS>(
    const Bits<WORD, CLSS>  bits)
    :   _word(bits.bits())
    {}

    constexpr
    Reg<WORD, CLSS>(
    const Mskd<WORD, CLSS>  mskd)
    :   _word(mskd.bits())
    {}


#ifdef REGBITS_COPY_CTOR  // implementing this impacts object passing performance
    constexpr
    Reg<WORD, CLSS>(
    const Reg<WORD, CLSS>   &other)
    :   _word(other._word)
    {}
#else
    Reg(const Reg<WORD, CLSS> &other) = default;
#endif

    // for passing constexpr instance by value without requiring storage
    Reg<WORD, CLSS> operator+()
    volatile const
    {
        return Reg<WORD, CLSS>(_word);
    }
    Reg<WORD, CLSS> operator+()
    const
    {
        return Reg<WORD, CLSS>(_word);
    }


    // for passing all bits off
    static constexpr
    Bits<WORD, CLSS> zero()
    {
        return Bits<WORD, CLSS>(0);
    }


    // assignments
    //
    void operator=(const WORD   word) volatile { _word = word; }
    void operator=(const WORD   word)          { _word = word; }

    void operator=(const Bits<WORD, CLSS>  bits) volatile { _word = bits._bits; }
    void operator=(const Bits<WORD, CLSS>  bits)          { _word = bits._bits; }

    void wrt(const Bits<WORD, CLSS> bits) volatile { _word = bits._bits; }
    void wrt(const Bits<WORD, CLSS> bits)          { _word = bits._bits; }

    void operator=(const Mskd<WORD, CLSS> &mskd) volatile { _word = mskd._bits; }
    void operator=(const Mskd<WORD, CLSS> &mskd)          { _word = mskd._bits; }

    void wrt(const Mskd<WORD, CLSS> &mskd) volatile { _word = mskd._bits; }
    void wrt(const Mskd<WORD, CLSS> &mskd)          { _word = mskd._bits; }


    void operator=(volatile const Reg<WORD, CLSS> &reg) volatile { _word = reg._word; }
    void operator=(const Reg<WORD, CLSS> &reg)          { _word = reg._word; }


    // accessor
    WORD word() volatile const { return _word; }
    WORD word()          const { return _word; }


    // bitwise operators
    //
    void operator|=(const Bits<WORD, CLSS>  bits) volatile
                    { _word |= bits._bits; }
    void operator|=(const Bits<WORD, CLSS>  bits)
                    { _word |= bits._bits; }

    void set(const Bits<WORD, CLSS> bits) volatile { _word |= bits._bits; }
    void set(const Bits<WORD, CLSS> bits)          { _word |= bits._bits; }

    void operator|=(const Mskd<WORD, CLSS>  &mskd) volatile
                    { _word |= mskd._bits; }
    void operator|=(const Mskd<WORD, CLSS>  &mskd)
                    { _word |= mskd._bits; }

    void set(const Mskd<WORD, CLSS> &mskd) volatile { _word |= mskd._bits; }
    void set(const Mskd<WORD, CLSS> &mskd)          { _word |= mskd._bits; }

    void operator-=(const Bits<WORD, CLSS>  bits) volatile
                    { _word &= ~bits._bits; }
    void operator-=(const Bits<WORD, CLSS>  bits)
                    { _word &= ~bits._bits; }

    void clr(const Bits<WORD, CLSS> bits) volatile { _word &= ~bits._bits; }
    void clr(const Bits<WORD, CLSS> bits)          { _word &= ~bits._bits; }

    void operator-=(const Mskd<WORD, CLSS>  &mskd) volatile
                    { _word &= ~mskd._bits; }
    void operator-=(const Mskd<WORD, CLSS>  &mskd)
                    { _word &= ~mskd._bits; }

    void clr(const Mskd<WORD, CLSS> &mskd) volatile { _word &= ~mskd._bits; }
    void clr(const Mskd<WORD, CLSS> &mskd)          { _word &= ~mskd._bits; }

    void operator^=(const Bits<WORD, CLSS>  bits) volatile
                    { _word ^= bits._bits; }
    void operator^=(const Bits<WORD, CLSS>  bits)
                    { _word ^= bits._bits; }

    void flp(const Bits<WORD, CLSS> bits) volatile { _word ^= bits._bits; }
    void flp(const Bits<WORD, CLSS> bits)          { _word ^= bits._bits; }

    void operator^=(const Mskd<WORD, CLSS>  &mskd) volatile
                    { _word ^= mskd._bits; }
    void operator^=(const Mskd<WORD, CLSS>  &mskd)
                    { _word ^= mskd._bits; }

    void flp(const Mskd<WORD, CLSS> &mskd) volatile { _word ^= mskd._bits; }
    void flp(const Mskd<WORD, CLSS> &mskd)          { _word ^= mskd._bits; }

    void operator/=(const Mskd<WORD, CLSS>  &mskd) volatile
                    { _word = (_word & ~mskd._mask) | mskd._bits; }
    void operator/=(const Mskd<WORD, CLSS>  &mskd)
                    { _word = (_word & ~mskd._mask) | mskd._bits; }

    void ins(const Mskd<WORD, CLSS> &mskd) volatile
                    { _word = (_word & ~mskd._mask) | mskd._bits; }
    void ins(const Mskd<WORD, CLSS> &mskd)
                    { _word = (_word & ~mskd._mask) | mskd._bits; }


    // extractors
    //
    Bits<WORD, CLSS> operator&(Bits<WORD, CLSS> bits) volatile const
                     { return Bits<WORD, CLSS>(_word & bits._bits); }
    Bits<WORD, CLSS> operator&(Bits<WORD, CLSS> bits)          const
                     { return Bits<WORD, CLSS>(_word & bits._bits); }

    Mskd<WORD, CLSS> operator&(Mskd<WORD, CLSS> mskd) volatile const
                     { return Mskd<WORD, CLSS>(_word & mskd._mask,
                                                       mskd._mask); }
    Mskd<WORD, CLSS> operator&(Mskd<WORD, CLSS> mskd)          const
                     { return Mskd<WORD, CLSS>(_word & mskd._mask,
                                                       mskd._mask); }

    WORD operator>>(const Shft<WORD, CLSS>  shft) volatile const
                    { return (_word & shft._mask) >> shft._pos._pos; }
    WORD operator>>(const Shft<WORD, CLSS>  shft)          const
                    { return (_word & shft._mask) >> shft._pos._pos; }

    WORD shifted(const Shft<WORD, CLSS> shft) volatile const
                    { return (_word & shft._mask) >> shft._pos._pos; }
    WORD shifted(const Shft<WORD, CLSS> shft)          const
                    { return (_word & shft._mask) >> shft._pos._pos; }


    // comparisons
    //
    bool is(const WORD  word) volatile const { return word == _word; }
    bool is(const WORD  word)          const { return word == _word; }

    bool is(const Bits<WORD, CLSS>  bits) volatile const
                    { return _word == bits._bits; }
    bool is(const Bits<WORD, CLSS>  bits)          const
                    { return _word == bits._bits; }

    bool is(const Mskd<WORD, CLSS>  mskd) volatile const
                    { return _word == mskd._bits; }
    bool is(const Mskd<WORD, CLSS>  mskd)          const
                    { return _word == mskd._bits; }


    bool all(const WORD word) volatile const
                    { return word & _word == _word; }
    bool all(const WORD word)          const
                    { return word & _word == _word; }

    bool all(const Bits<WORD, CLSS> bits) volatile const
                    { return (_word & bits._bits) == bits._bits; }
    bool all(const Bits<WORD, CLSS> bits)          const
                    { return (_word & bits._bits) == bits._bits; }

    bool all(
    const Bits<WORD, CLSS>  mask,
    const Bits<WORD, CLSS>  bits)
    volatile const
    {
        return (_word & mask._bits) == bits._bits;
    }
    bool all(
    const Bits<WORD, CLSS>  mask,
    const Bits<WORD, CLSS>  bits)
    const
    {
        return (_word & mask._bits) == bits._bits;
    }

    bool all(const Mskd<WORD, CLSS> mskd) volatile const
                    { return (_word & mskd._mask) == mskd._bits; }
    bool all(const Mskd<WORD, CLSS> mskd)          const
                    { return (_word & mskd._mask) == mskd._bits; }


    bool any(const WORD word) volatile const
                    { return static_cast<bool>(word & _word); }
    bool any(const WORD word)          const
                    { return static_cast<bool>(word & _word); }

    bool any(const Bits<WORD, CLSS> bits) volatile const
                    { return static_cast<bool>(_word & bits._bits); }
    bool any(const Bits<WORD, CLSS> bits)          const
                    { return static_cast<bool>(_word & bits._bits); }


    bool operator<(const Mskd<WORD, CLSS>   &mskd) volatile const
                    { return (_word & mskd._mask) < mskd._bits; }
    bool operator<(const Mskd<WORD, CLSS>   &mskd)          const
                    { return (_word & mskd._mask) < mskd._bits; }

    bool operator<=(const Mskd<WORD, CLSS>  &mskd) volatile const
                    { return (_word & mskd._mask) <= mskd._bits; }
    bool operator<=(const Mskd<WORD, CLSS>  &mskd)          const
                    { return (_word & mskd._mask) <= mskd._bits; }

    bool operator>(const Mskd<WORD, CLSS>   &mskd) volatile const
                    { return (_word & mskd._mask) > mskd._bits; }
    bool operator>(const Mskd<WORD, CLSS>   &mskd)          const
                    { return (_word & mskd._mask) > mskd._bits; }

    bool operator>=(const Mskd<WORD, CLSS>  &mskd) volatile const
                    { return (_word & mskd._mask) >= mskd._bits; }
    bool operator>=(const Mskd<WORD, CLSS>  &mskd)          const
                    { return (_word & mskd._mask) >= mskd._bits; }


  protected:
    WORD    _word;


  private:
    // do not implement this, even as private
    // operator WORD() {}

};  // template<typename BITS, typename MSKD> class Reg




// macro for generating functions returning Bits (constexpr and non-)
// assumes pos_t and bits_ have been typedef'd/using'd
//
#define REGBITS_BITS_RANGE(CLASS, CONSTEXPR_NAME, RUNTIME_NAME, WORD) \
template<unsigned BIT_NUM> static constexpr bits_t CONSTEXPR_NAME() \
    { \
    static_assert(BIT_NUM < sizeof(WORD) * 8, \
                  CLASS "::" #CONSTEXPR_NAME "<BIT_NUM> out of range"); \
    return bits_t(1, pos_t(BIT_NUM)); \
} \
\
static const bits_t RUNTIME_NAME( \
const unsigned  bit_num) \
{ \
    return bits_t(1, pos_t(bit_num)); \
} \
\
static bool RUNTIME_NAME##_valid( \
const unsigned  bit_num) \
{ \
    return bit_num < sizeof(WORD) * 4; \
}


// macro for generating functions returning Mskd (constexpr and non-)
// assumes shft_t and mskd_t have been typedef'd/using'd
//
#define REGBITS_MSKD_RANGE(CLASS, CONSTEXPR_NAME, RUNTIME_NAME, MASK, POS, LIMIT) \
static constexpr shft_t     CONSTEXPR_NAME##_SHFT = shft_t(MASK, POS);   \
\
template<unsigned BITS> static constexpr mskd_t CONSTEXPR_NAME() \
    { \
    static_assert(BITS <= (LIMIT), \
                  CLASS "::" #CONSTEXPR_NAME "<BITS> out of range"); \
    return mskd_t(MASK, BITS, POS); \
} \
\
static const mskd_t RUNTIME_NAME( \
const unsigned  bits) \
{ \
    return mskd_t(MASK << POS.pos(), bits << POS.pos()); \
} \
\
static bool RUNTIME_NAME##_valid( \
const unsigned  bits) \
{ \
    return bits <= (LIMIT); \
}


// macro for generating functions returning array member (constexpr and non-)
#define REGBITS_ARRAY_RANGE(CLASS, CONSTEXPR_NAME, RUNTIME_NAME, DATATYPE, ARRAY, LIMIT) \
template<unsigned INDEX> volatile DATATYPE& CONSTEXPR_NAME() \
    volatile { \
    static_assert(INDEX <= (LIMIT), \
                  CLASS "::" #CONSTEXPR_NAME "<INDEX> out of range"); \
    return ARRAY[INDEX]; \
} \
\
volatile DATATYPE& RUNTIME_NAME( \
const unsigned  index) \
volatile { \
    return ARRAY[index]; \
} \
\
static bool RUNTIME_NAME##_valid( \
const unsigned  index) \
{ \
    return index <= (LIMIT); \
}

}  // namespace regbits

#endif  // ifndef regbits_hxx
