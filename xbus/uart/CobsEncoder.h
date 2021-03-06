/*
 * APX Autopilot project <http://docs.uavos.com>
 *
 * Copyright (c) 2003-2020, Aliaksei Stratsilatau <sa@uavos.com>
 * All rights reserved
 *
 * This file is part of APX Shared Libraries.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */
#pragma once

#include <crc.h>
#include <QueueBuffer.h>

#include "SerialCodec.h"

template<size_t _buf_size, typename T = uint8_t, T _esc = 0>
class CobsEncoder : public SerialEncoder
{
public:
    explicit CobsEncoder()
        : SerialEncoder(_buf, _buf_size)
    {}

    //    using QueueBuffer<_buf_size, T>::tail;
    //    using QueueBuffer<_buf_size, T>::size;
    //    using QueueBuffer<_buf_size, T>::total;
    //    using QueueBuffer<_buf_size, T>::read_ptr;

    //    using QueueBuffer<_buf_size, T>::head;
    //    using QueueBuffer<_buf_size, T>::skip_read;

    //write and encode data to fifo
    size_t encode(const void *src, size_t sz) override
    {
        if (space() < (sz + (2 + 4 + sizeof(uint16_t)))) //estimate
            return 0;

        size_t head_s = head();
        size_t size_s = size();
        do {
            // start block
            T *code_ptr = write_ptr();
            T code = 1;
            if (!write(code))
                break;

            // data
            const T *ptr = static_cast<const T *>(src);
            size_t cnt = sz + 2;
            uint16_t crc16 = apx::crc32(src, sz);
            while (cnt) {
                if (code != 0xFF) {
                    T c;
                    switch (cnt) {
                    default:
                        c = *ptr++;
                        break;
                    case 1:
                        c = crc16 >> 8;
                        break;
                    case 2:
                        c = crc16;
                        break;
                    }
                    cnt--;
                    if (c != _esc) {
                        if (!write(c))
                            break;
                        code++;
                        continue;
                    }
                }
                // finish block
                *code_ptr = code;
                // start block
                code_ptr = write_ptr();
                code = 1;
                if (!write(code))
                    break;
            }
            if (cnt) //error
                break;

            // finish block
            *code_ptr = code;

            //append packet delimiter
            if (!write(_esc))
                break;

            // all encoded
            return size() - size_s;
        } while (0);
        //error - fifo overflow
        pop_head(head_s);
        return 0;
    }

    inline size_t read_encoded(void *dest, size_t sz) override
    {
        return SerialCodec::read(dest, sz);
    }
    //    inline size_t size() override
    //    {
    //        return QueueBuffer<_buf_size, T>::size();
    //    }
    //    inline void reset() override
    //    {
    //        QueueBuffer<_buf_size, T>::reset();
    //    }

private:
    uint8_t _buf[_buf_size];

    //    using QueueBuffer<_buf_size, T>::space;
    //    using QueueBuffer<_buf_size, T>::pop_head;
    //    using QueueBuffer<_buf_size, T>::write;
    //    using QueueBuffer<_buf_size, T>::write_ptr;
    //    using QueueBuffer<_buf_size, T>::read;
};

/*template<size_t _buf_size, typename T = uint8_t, T _esc = 0>
class CobsEncoder2 : private QueueBuffer<_buf_size, T>, public SerialEncoder
{
public:
    using QueueBuffer<_buf_size, T>::head;
    using QueueBuffer<_buf_size, T>::tail;
    using QueueBuffer<_buf_size, T>::size;
    using QueueBuffer<_buf_size, T>::total;
    using QueueBuffer<_buf_size, T>::read_ptr;
    using QueueBuffer<_buf_size, T>::skip_read;

    //write and encode data to fifo
    size_t encode(const void *src, size_t sz) override
    {
        if (!start(sz))
            return 0;

        const T *ptr = static_cast<const T *>(src);

        uint8_t crc = 0;
        while (sz--) {
            crc ^= *ptr;
            if (!push(*ptr++))
                return 0;
        }
        if (!push(crc))
            return 0;

        return finish();
    }

    bool start(size_t sz)
    {
        if (space() < (sz + 1 + 2)) //estimate
            return false;

        _head_s = head();
        _size_s = size();

        // start block
        code_ptr = write_ptr();
        code = 1;
        if (!write(code))
            return false;
        return true;
    }
    size_t finish()
    {
        // finish block
        *code_ptr = code;

        //append packet delimiter
        if (!write(_esc)) {
            cancel();
            return 0;
        }

        // all encoded
        return size() - _size_s;
    }
    void cancel()
    {
        pop_head(_head_s);
    }
    bool push(T c)
    {
        do {
            if (code != 0xFF) {
                if (c != _esc) {
                    if (!write(c))
                        break;
                    code++;
                    return true;
                }
            }
            // finish block
            *code_ptr = code;
            // start block
            code_ptr = write_ptr();
            code = 1;
            if (!write(code))
                break;
            if (c == _esc)
                return true;
            return push(c);
        } while (0);
        cancel();
        return false;
    }

    inline size_t read_encoded(void *dest, size_t sz) override
    {
        return QueueBuffer<_buf_size, T>::read(dest, sz);
    }
    inline size_t size() override
    {
        return QueueBuffer<_buf_size, T>::size();
    }
    inline void reset() override
    {
        QueueBuffer<_buf_size, T>::reset();
    }

private:
    size_t _head_s;
    size_t _size_s;

    T *code_ptr;
    T code;

    using QueueBuffer<_buf_size, T>::space;
    using QueueBuffer<_buf_size, T>::pop_head;
    using QueueBuffer<_buf_size, T>::write;
    using QueueBuffer<_buf_size, T>::write_ptr;
    using QueueBuffer<_buf_size, T>::read;
};*/
