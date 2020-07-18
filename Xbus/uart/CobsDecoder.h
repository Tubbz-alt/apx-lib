#pragma once

#include <QueueBuffer.h>
#include <crc.h>

#include "SerialCodec.h"

template<size_t _buf_size, typename T = uint8_t, T _esc = 0>
class CobsDecoder : private QueueBuffer<_buf_size, T>, public SerialDecoder
{
public:
    using QueueBuffer<_buf_size, T>::head;

    //write and encode data to fifo
    ErrorType decode(const void *src, size_t sz) override
    {
        if (sz == 0)
            return DataDropped;
        //restore fifo with partially read packet
        if (_rcnt > 0) {
            push_head(_head_pending_s);
        }

        ErrorType ret = DataAccepted;
        const T *ptr = static_cast<const T *>(src);
        size_t cnt = sz;
        while (cnt--) {
            T c = *ptr++;
            if (c == _esc) {
                //packet delimiter
                if (_copy) {
                    error();
                    return ErrorSize;
                }
                ret = check_packet();
                continue;
            }
            if (_copy != 0) {
                if (!write_decoded_byte(c)) {
                    error();
                    return ErrorOverflow;
                }
            } else {
                if (_code != 0xFF) {
                    if (!write_decoded_byte(_esc)) {
                        error();
                        return ErrorOverflow;
                    }
                }
                _copy = _code = c;
            }
            _copy--;
        }
        if (_rcnt > 0) {
            //block fifo until packet read done
            _head_pending_s = head();
            pop_head(_head_s);
        }
        return ret; //always accept all bytes
    }

    inline size_t read_decoded(void *dest, size_t sz) override
    {
        sz = QueueBuffer<_buf_size, T>::read_packet(dest, sz);
        if (sz <= sizeof(uint16_t))
            return 0;
        sz -= sizeof(uint16_t);
        const uint8_t *p = static_cast<const uint8_t *>(dest) + sz;
        uint16_t packet_crc16 = p[0] | (p[1] << 8);
        uint16_t crc16 = apx::crc32(dest, sz);
        if (packet_crc16 != crc16) {
            return 0;
        }
        return sz;
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
    size_t _head_pending_s;
    size_t _head_s;
    size_t _rcnt{0};

    T _code{0xFF};
    T _copy{0};

    inline void restart()
    {
        _code = 0xFF;
        _copy = 0;
        _rcnt = 0;
    }
    inline void error()
    {
        if (_rcnt > 0) {
            _rcnt = 0;
            pop_head(_head_s);
        }
        restart();
    }
    bool write_decoded_byte(const T &c)
    {
        if (_rcnt == 0) {
            _head_s = head();
            if (!write_word(0))
                return false;
        }
        if (!write(c))
            return false;
        _rcnt++;
        return true;
    }
    inline ErrorType check_packet()
    {
        if (_rcnt < sizeof(uint16_t)) { //no data
            error();
            return ErrorSize;
        }

        //frame received...
        size_t h = head();
        pop_head(_head_s);
        write_word(_rcnt);
        push_head(h);
        restart();
        return DataAccepted;
    }

    using QueueBuffer<_buf_size, T>::space;
    using QueueBuffer<_buf_size, T>::push_head;
    using QueueBuffer<_buf_size, T>::pop_head;
    using QueueBuffer<_buf_size, T>::pop_one;
    using QueueBuffer<_buf_size, T>::write;
    using QueueBuffer<_buf_size, T>::write_word;
};
