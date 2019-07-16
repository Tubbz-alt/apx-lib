#pragma once
#include <inttypes.h>
#include <sys/types.h>

#include <array>
#include <string>

#include "endian.h"


class XbusWriter
{
public:
    explicit XbusWriter(uint8_t *p)
        : msg(p)
        , pos(0)

    {}
    explicit XbusWriter(uint8_t &p)
        : msg(&p)
        , pos(0)

    {}

    inline void reset()
    {
        pos = 0;
    }


    template<typename _T>
    void write(const _T data);

    template<class _T, size_t _Size>
    void write(const std::array<_T, _Size> &data);

    template<typename _T, typename _Tin>
    inline void write(const _Tin data)
    {
        write(static_cast<_T>(data));
    }


    template<typename _T>
    inline void operator<< (const _T data)
    {write(data);}

    template<class _T, size_t _Size>
    inline void operator<< (const std::array<_T, _Size> &data)
    {write(data);}

private:
    uint8_t *msg;
    uint16_t pos;

    template<typename _T, typename _Tin>
    inline void set_data(_T &buf, _Tin data);
};

// implementation

template<typename _T, typename _Tin>
void XbusWriter::set_data(_T &buf, _Tin data)
{
    if (std::is_floating_point<_Tin>::value) {
        buf = *static_cast<const _T *>(static_cast<const void *>(&data));
    } else {
        buf = data;
    }

    // htoleXX functions may be empty macros,
    // switch will be optimized-out
    switch (sizeof(_T)) {
    case 2:
        buf = htole16(buf);
        break;

    case 4:
        buf = htole32(buf);
        break;

    case 8:
        buf = htole64(buf);
        break;

    default:
        assert(false);
    }

    memcpy(&msg[pos], &buf, sizeof(buf));
}

template<typename _T>
void XbusWriter::write(const _T data)
{
    switch (sizeof(_T)) {
    case 1:
        msg[pos] = data;
        break;

    case 2:
        uint16_t data_le16;
        set_data(data_le16, data);
        break;

    case 4:
        uint32_t data_le32;
        set_data(data_le32, data);
        break;

    case 8:
        uint64_t data_le64;
        set_data(data_le64, data);
        break;

    default:
        assert(false);
    }
    pos += sizeof(_T);
}

template<class _T, size_t _Size>
void XbusWriter::write(const std::array<_T, _Size> &data)
{
    for (auto &v : data) {
        *this << v;
    }
}
