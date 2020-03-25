#pragma once

#include <common/visibility.h>
#include <sys/types.h>

#include "XbusStreamReader.h"
#include "XbusStreamWriter.h"

namespace xbus {

typedef uint16_t pid_raw_t;

constexpr const size_t size_packet_max = 512;

enum sub_e : uint8_t {
    sub_primary = 0,
    sub_secondary,
    sub_failsafe,
    sub_ext = 5,      // extended index follows
    sub_response = 6, // response not request
    sub_request = 7,  // request not response
};

// Packet identifier [16 bits]
#pragma pack(1)
union pid_s {
    pid_raw_t _raw;

    struct
    {
        uint16_t uid : 11; // dictionary

        uint8_t sub : 3; // [0,7] sub index, 1=request not response
        uint8_t seq : 2; // sequence counter
    };

    static inline uint16_t psize()
    {
        return sizeof(pid_raw_t);
    }
    inline void read(XbusStreamReader *s)
    {
        *s >> _raw;
    }
    inline void write(XbusStreamWriter *s) const
    {
        *s << _raw;
    }
};
static_assert(sizeof(pid_s) == 2, "pid_s size error");

#pragma pack()

} // namespace xbus
