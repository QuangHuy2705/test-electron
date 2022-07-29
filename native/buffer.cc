#include "buffer.h"

void Buffer::read_codec(AVCodecParameters* p) {
  p->codec_type = read_cast<AVMediaType, int32_t>();
  p->codec_id = read_cast<AVCodecID, uint32_t>();
  p->codec_tag = read<uint32_t>();
  p->bit_rate = read<int64_t>();
  p->bits_per_coded_sample = read<int32_t>();
  p->bits_per_raw_sample = read<int32_t>();
  p->profile = read<int32_t>();
  p->level = read<int32_t>();
  p->extradata_size = read<int32_t>();
  p->format = read<int32_t>();
  p->width = read<int32_t>();
  p->height = read<int32_t>();
  p->field_order = read_cast<AVFieldOrder, uint32_t>();
  p->color_range = read_cast<AVColorRange, uint32_t>();
  p->color_primaries = read_cast<AVColorPrimaries, uint32_t>();
  p->color_trc = read_cast<AVColorTransferCharacteristic, uint32_t>();
  p->color_space = read_cast<AVColorSpace, uint32_t>();
  p->chroma_location = read_cast<AVChromaLocation, uint32_t>();
  p->sample_aspect_ratio = read_rational();
  p->video_delay = read<int32_t>();
  if (p->extradata_size) {
    p->extradata = reinterpret_cast<uint8_t*>(av_mallocz(p->extradata_size + AV_INPUT_BUFFER_PADDING_SIZE));
    if (!p->extradata) {
      throw StreamError("cannot init extra data");
    }

    read(p->extradata, p->extradata_size);
  }
}

void Buffer::read_packet(AVPacket* p) {
  p->pts = read<int64_t>();
  p->dts = p->pts; // read<int64_t>();
  p->size = read<int32_t>();
  // p->stream_index = read<int32_t>();
  p->flags = read<int32_t>();
  // p->side_data_elems = read<int32_t>();
  // p->duration = read<int64_t>();
  // p->pos = read<int64_t>();
  p->side_data_elems = 0;

  if (p->size) {
    auto d = reinterpret_cast<uint8_t*>(av_mallocz(p->size + AV_INPUT_BUFFER_PADDING_SIZE));
    if (!d) {
      throw StreamError("cannot init packet data");
    }

    read(d, p->size);
    av_packet_from_data(p, d, p->size);
  }

  // for (int i = 0; i < p->side_data_elems; ++i)
  // {
  //   auto type = read_cast<AVPacketSideDataType, uint32_t>();
  //   auto size = read<int32_t>();
  //   auto d = reinterpret_cast<uint8_t *>(av_mallocz(size));
  //   if (!d)
  //   {
  //     throw StreamError("cannot init side data");
  //   }
  //   read(d, size);
  //   av_packet_add_side_data(p, type, d, size);
  // }
}
