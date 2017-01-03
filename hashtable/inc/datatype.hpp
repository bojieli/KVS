#pragma pack (1)
struct Data32 {
  uchar data[32];
};

#pragma pack (1)
typedef struct DmaContext {
  uchar id;
  ushort size;
}DmaContext;

#pragma pack (1)
typedef struct DmaReadReqWithId {
  DMA_ReadReq req;
  uchar id;
}DmaReadReqWithId;

#pragma pack (1)
typedef struct PutReq {
  uchar net_meta;
  uchar key_size;
  ulong4 key;
  ushort val_size;
  ulong4 val;
  uint hash1;
  uint hash2;
  bool has_last;
}PutReq;

#pragma pack (1)
typedef struct GetReq {
  uchar net_meta;
  uchar key_size;
  ulong4 key;
  uint hash1;
  ushort hash2;
  bool has_last;
}GetReq;

#pragma pack (1)
typedef struct DelReq {
  uchar net_meta;
  uchar key_size;
  ulong4 key;
  uint hash1;
  ushort hash2;
  bool has_last;
  uint last_line_addr;
  ulong4 last_half_line_data;
  bool last_has_last;
}DelReq;

#pragma pack (1)
typedef struct AddReq {
  bool dummy;
}AddReq;

#pragma pack (1)
typedef struct GetRes {
  uchar net_meta;
  bool found;
  uchar key_size;
  ulong4 key;
  ushort val_size;
  ulong4 val;
}GetRes;

#pragma pack (1)
typedef struct DelRes {
  uchar net_meta;
  bool found;
  uchar key_size;
  ulong4 key;
}DelRes;

#pragma pack (1)
typedef struct PutRes {
  uchar net_meta;
  bool found;
  uchar key_size;
  ulong4 key;
}PutRes;

#pragma pack (1)
typedef struct Ulong16 {
  ulong8 x;
  ulong8 y;
  bool valid2;
}Ulong16;

#pragma pack (1)
typedef struct PutOfflineType {
  PutReq req;
  ulong8 line;
  uchar put_idx;
}PutOfflineType;

#pragma pack (1)
typedef struct PutNewlineType {
  PutReq req;
  ulong4 half_line;
}PutNewlineType;
