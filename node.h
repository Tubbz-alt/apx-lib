#ifndef _NODE_H_
#define _NODE_H_
#include <inttypes.h>
#include <sys/types.h>
//=============================================================================
#define BUS_MAX_PACKET          2048    //2k MAX
//=============================================================================
// node information, dynamically assigned values
typedef uint8_t _node_sn[12]; //chip serial number
typedef uint8_t _node_name[16]; //device name string
typedef uint8_t _node_version[16]; //fw version string
//-------------------------------------------------
// firmware information, saved in FLASH section [CONFIG]
typedef struct {
  _node_name    name;
  _node_version version;
  uint8_t       crc;
  uint32_t      size;
}__attribute__((packed)) _fw_info;
// node information, filled by hwInit, saved in RAM
typedef struct{
  uint8_t     err_cnt;        // errors counter
  uint8_t     can_rxc;        // CAN received packets counter
  uint8_t     can_adr;        // CAN address
  uint8_t     can_err;        // CAN errors counter
  uint8_t     dump[8];        // error dump
}_node_status;
typedef struct{
  _node_sn      sn;     //serial number
  _fw_info      fw;     //firmware info
  _node_status  status; //dynamic status
}__attribute__((packed)) _node_info;
//=============================================================================
// bus packet structure:
// <tag> is an optional byte (stored in conf)
// <tag> can be any value 7bits (<0x7F)
// <tag>=0x7F is reserved for 'shiva'
// MSB in <tag> flags service packet
// all service packets start with _node_sn in <data>
// service packets filtered by _node_sn, or 0x00000 for broadcast
typedef struct{
  uint8_t       id;   //<var_idx>
  union {
    uint8_t     data[BUS_MAX_PACKET-1];   //payload
    struct{
      _node_sn  sn;     //filter for service
      uint8_t   cmd;    //service command
      uint8_t   data[BUS_MAX_PACKET-1-sizeof(_node_sn)-1];   //service data
    }srv;
    struct{
      uint8_t   tag;    //user tag
      uint8_t   packet[BUS_MAX_PACKET-1-1];   //tagged packet
    }tagged;
  };
}__attribute__((packed)) _bus_packet;
#define bus_packet_size_hdr             (1)
#define bus_packet_size_hdr_srv         (bus_packet_size_hdr+sizeof(_node_sn)+1)
#define bus_packet_size_hdr_tagged      (bus_packet_size_hdr+1)
//=============================================================================
// default bus commands (service packets)
// all service packets addressed by _node_sn
enum{
  apc_ack=0,    //acknowledge - sent back in response to commands
  //------------------
  //system commands
  apc_info,     //return _fw_info
  apc_nstat,    //return _node_name + _node_status
  apc_debug,    //debug message
  apc_reboot,   //reset/reboot node
  apc_mute,     //stop sending data (sensors)

  //------------------
  //conf
  apc_fields=32,        //return conf descriptor
  apc_commands,         //return node commands descrptor
  apc_rconf,            //return _node_conf structure
  apc_wconf,            //save _node_conf
  apc_econf,            //reset conf to defaults
  //future use
  apc_rp,               //return _node_conf parameter <num>
  apc_wp,               //save _node_conf parameter <num>,<data>
  
  //------------------
  //standard commands
  apc_data=64,          //send/receive some data [portNo]

  //------------------
  //user commands
  apc_user=100,
  apc_loader=0xFF
};
//=============================================================================
// Loader packet data (packet.srv.cmd=apc_loader):
// <ldc_COMMAND>,<data...>
// loader must reply to standard command 'apc_ID'
//------------------------------------------------------------------------------
typedef enum {
  ldc_init=0,   // start loader, re: <_ldc_file> =flash size
  ldc_file,     // set file info <_ldc_file>, re: <_ldc_file>
  ldc_write,    // write file data <_ldc_write>, re: <_ldc_write::hdr>
} _ldr_cmd;
//loader data
typedef struct{
  uint32_t      start_address;
  uint32_t      size;        // flash size in kB
  uint8_t       xor_crc;     // all file data XORed
}__attribute__((packed)) _ldc_file;
typedef struct{
  struct{
    uint32_t    start_address;
    uint16_t    data_size;
  }__attribute__((packed)) hdr;
  uint8_t       data[256];
}__attribute__((packed)) _ldc_write;
//=============================================================================
// NODE CONF typedefs
//=============================================================================
typedef uint8_t   _ft_option;
typedef uint16_t  _ft_varmsk;
typedef uint16_t  _ft_uint;
typedef float     _ft_float;
typedef uint8_t   _ft_byte;
typedef uint8_t   _ft_string[16];
typedef struct{
  _ft_float x;
  _ft_float y;
  _ft_float z;
}__attribute__((packed)) _vec;
typedef _vec      _ft_vec;
typedef struct{
  _ft_varmsk varmsk;  //var idx with mask
  int8_t  mult;       //multiplier (x0.1) -127[-12.7]..+127[+12.7]
  int8_t  diff;       //differential multiplier (x0.01+1) for pos/neg var value
  uint8_t speed;      //speed of change (x0.1) 0..25.5
  uint8_t pwm_ch;     //pwm channel number 0...x
}__attribute__((packed)) _ctr;
typedef _ctr      _ft_ctr;
typedef struct {
  int8_t zero;          //pwm zero shift -127[-1]..+127[+1]
  int8_t max;           //pwm maximum -127[-1]..+127[+1]
  int8_t min;           //pwm minimum -127[-1]..+127[+1]
}__attribute__((packed)) _pwm;
typedef _pwm      _ft_pwm;
typedef struct {
  _ft_varmsk varmsk;  //var idx with mask
  _ft_byte opt;       //option, inversion=1
}__attribute__((packed)) _gpio;
typedef _gpio     _ft_gpio;
//-----------------------------------------------------------------------------
typedef struct {
  _ft_option protocol;    //protocol
  _ft_float  baudrate;    //baud rate for some protocols
  _ft_byte   port_id;     //port_id for 'data' packets
}__attribute__((packed)) _serial;
typedef _serial   _ft_serial;
//-----------------------------------------------------------------------------
typedef struct {
  _ft_option type;    //input capture type
  _ft_varmsk varmsk;  //var idx with mask
}__attribute__((packed)) _capture;
typedef _capture  _ft_capture;
//-----------------------------------------------------------------------------
typedef enum{
  ft_option=0,
  ft_varmsk,
  ft_uint,
  ft_float,
  ft_byte,
  ft_string,
  ft_vec,
  ft_ctr,
  ft_pwm,
  ft_gpio,
  ft_serial,
  ft_capture,
  //---------
  ft_cnt
}_node_ft;
//=============================================================================
#endif

