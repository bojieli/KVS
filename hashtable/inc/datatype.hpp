#pragma pack (1)
struct Data32 {
  uchar data[32];
};

typedef struct DmaContext {
  uchar id;
  ushort size;
}DmaContext;

typedef struct DmaReadReqWithId {
  DMA_ReadReq req;
  uchar id;
}DmaReadReqWithId;

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

typedef struct GetReq {
  bool is_array_first;
  uchar net_meta;
  uchar key_size;
  ulong4 key;
  uint hash1;
  ushort hash2;
  bool has_last;
}GetReq;

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

typedef struct AddReq {
  bool dummy;
}AddReq;

typedef struct GetRes {
  bool is_array_first;
  uchar net_meta;
  bool found;
  uchar key_size;
  ulong4 key;
  ushort val_size;
  ulong4 val;
}GetRes;

typedef struct DelRes {
  uchar net_meta;
  bool found;
  uchar key_size;
  ulong4 key;
}DelRes;

typedef struct PutRes {
  uchar net_meta;
  bool found;
  uchar key_size;
  ulong4 key;
}PutRes;

typedef struct ArrayGetReqInfo {
  uchar net_meta;
  ulong4 key;
  ushort cnt;
  uchar key_size;
}ArrayGetReqInfo;

typedef struct Ulong16 {
  ulong8 x;
  ulong8 y;
  bool valid2;
}Ulong16;

typedef struct PutOfflineType {
  PutReq req;
  ulong8 line;
  uchar put_idx;
}PutOfflineType;

typedef struct PutNewlineType {
  PutReq req;
  ulong4 half_line;
}PutNewlineType;

typedef struct GetOfflineType {
  uchar net_meta;
  bool is_array_first;
  ushort size;
}GetOfflineType;
