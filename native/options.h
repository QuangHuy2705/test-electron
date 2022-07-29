#pragma once

struct StreamVideoOptions
{
  int dec_thread_count = 0; // set if > 0
  //  1: FF_THREAD_FRAME, decode more than one frame at once
  //  2: FF_THREAD_SLICE, decode more than one part of a single frame at once
  int dec_thread_type = 0; // set if > 0
  int dec_max_data_count = 0;
  int dec_max_frame_count = 0;
  double speed = 1;
};
