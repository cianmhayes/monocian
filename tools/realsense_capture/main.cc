
#include <glog/logging.h>
#include <json/json.h>
#include <chrono>
#include <ctime>
#include <fstream>
#include <iostream>
#include <librealsense2/rs.hpp>
#include <tuple>

#include "av/video_encoder.h"
#include "av/video_encoding_queue.h"
#include "az/buffered_blob_writer.h"
#include "base/storage/broadcast_writer.h"
#include "base/storage/file_writer.h"
#include "ogl/constants.h"
#include "ogl/full_screen_video.h"
#include "ogl/window.h"

namespace ogl {

template <>
const void* GetFrameData<rs2::video_frame>(const rs2::video_frame& frame) {
  return frame.get_data();
}

template <>
int GetFrameWidth<rs2::video_frame>(const rs2::video_frame& frame) {
  return frame.get_width();
}

template <>
int GetFrameHeight<rs2::video_frame>(const rs2::video_frame& frame) {
  return frame.get_height();
}

}  // namespace ogl

std::string get_date_string() {
  std::time_t now =
      std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
  char buf[100] = {0};
  std::strftime(buf, sizeof(buf), "%Y-%m-%d-%H-%M", std::localtime(&now));
  return std::string(buf);
}

struct FerrySettings {
  std::string connection_string;
  std::string container_name;
  std::string blob_root;
  bool write_to_file = false;
  bool write_to_service = false;
  int depth_bitrate_bps = 0;
  int color_bitrate_bps = 0;
  bool valid_settings = false;
};

FerrySettings ReadSettings(const std::string& settings_path) {
  Json::Value root;
  std::ifstream ifs;
  ifs.open(settings_path);

  Json::CharReaderBuilder builder;
  JSONCPP_STRING errs;
  if (!parseFromStream(builder, ifs, &root, &errs)) {
    std::cout << errs << std::endl;
    return {};
  }
  FerrySettings settings;
  settings.connection_string = root["connection_string"].asString();
  settings.container_name = root["container_name"].asString();
  settings.blob_root = root["blob_root"].asString();
  settings.write_to_file = root["write_to_file"].asBool();
  settings.write_to_service = root["write_to_service"].asBool();
  settings.depth_bitrate_bps = root["depth_bitrate_bps"].asInt();
  settings.color_bitrate_bps = root["color_birate_bps"].asInt();
  settings.valid_settings = true;

  if (settings.depth_bitrate_bps <= 0 || settings.color_bitrate_bps <= 0) {
    settings.valid_settings = false;
    return settings;
  }

  if (settings.write_to_service &&
      !(!settings.connection_string.empty() ||
        !settings.container_name.empty() || !settings.blob_root.empty())) {
    settings.valid_settings = false;
    return settings;
  }

  return settings;
};

int main(int argc, char* argv[]) {
  google::InitGoogleLogging(argv[0]);

  FerrySettings settings;
  if (argc > 1) {
    std::cout << "Reading settings from parameter." << std::endl;
    settings = ReadSettings(argv[1]);
  }

  if (!settings.valid_settings) {
    std::cout << "Reading settings from default settings." << std::endl;
    settings = ReadSettings("secret.json");
  }

  if (!settings.valid_settings) {
    std::cout << "Missing a required parameter." << std::endl;
    return 1;
  }

  ogl::Window app(1280, 720, "Ferry", true);

  rs2::pipeline pipe;
  rs2::config config;
  config.disable_all_streams();
  config.enable_stream(rs2_stream::RS2_STREAM_COLOR);
  config.enable_stream(rs2_stream::RS2_STREAM_DEPTH);
  pipe.start(config);

  std::string timestamp = get_date_string();
  std::vector<std::unique_ptr<base::storage::Writer>> depth_writers = {};
  std::vector<std::unique_ptr<base::storage::Writer>> color_writers = {};
  if (settings.write_to_file) {
    depth_writers.push_back(
        std::make_unique<base::storage::FileWriter>(("depth_" + timestamp + ".asf").c_str()));
    color_writers.push_back(
        std::make_unique<base::storage::FileWriter>(("color_" + timestamp + ".asf").c_str()));
  }
  if (settings.write_to_service) {
    std::string depth_blob_name =
        settings.blob_root + "/" + timestamp + "/depth.asf";
    std::string color_blob_name =
        settings.blob_root + "/" + timestamp + "/color.asf";
    depth_writers.push_back(std::make_unique<az::BufferedBlobWriter>(
        settings.connection_string, settings.container_name, depth_blob_name,
        2 * 1024 * 1024));
    color_writers.push_back(std::make_unique<az::BufferedBlobWriter>(
        settings.connection_string, settings.container_name, color_blob_name,
        2 * 1024 * 1024));
  }

  av::VideoEncodingQueue depth_queue(
      std::make_unique<base::storage::BroadcastWriter>(std::move(depth_writers)), 30, 848, 480,
      settings.depth_bitrate_bps);
  av::VideoEncodingQueue color_queue(
      std::make_unique<base::storage::BroadcastWriter>(std::move(color_writers)), 30, 1280,
      720, settings.color_bitrate_bps);

  depth_queue.Start();
  color_queue.Start();

  bool show_video = true;
  bool* show_video_ptr = &show_video;

  app.AddKeyReleasedCallback(
      [&show_video_ptr](ogl::Window* window, int32_t key) {
        switch (key) {
          case OGL_KEY_ESCAPE:
            window->SetShouldClose(true);
            break;
          case OGL_KEY_D:
            *show_video_ptr = !(*show_video_ptr);
            break;
        }
      });

  rs2::colorizer colourizer;
  ogl::FullScreenVideo video(&app);
  while (app.FrameStart()) {
    rs2::frameset frames = pipe.wait_for_frames();
    rs2::depth_frame df = frames.get_depth_frame();
    rs2::video_frame vf = frames.get_color_frame();

    if (show_video) {
      video.RenderFrame(vf, ogl::FrameFormat::RGB_8);
    } else {
      video.RenderFrame(df.apply_filter(colourizer).as<rs2::video_frame>(),
                        ogl::FrameFormat::RGB_8);
    }

    if (df.get_data_size() > 0) {
      const uint8_t* raw_depth_data =
          static_cast<const uint8_t*>(df.get_data());
      int source_data_size = df.get_data_size();
      int data_stride = df.get_stride_in_bytes();
      int depth_width = df.get_width();
      int depth_height = df.get_height();
      std::vector<uint8_t> depth_data(depth_width * depth_height * 3);
      for (int i = 0; i < depth_width * depth_height; i += 2) {
        // We want to prepare an RGB frame from 16bit depth data so that it will
        // preserve as much detail as possible when converted to YUV.
        // Given a pair of 16bit depth pixels, split each into most and least
        // significant bytes. Set the green channel to the most significant bit
        // which will become Y. Then the even least significant bytes will form
        // the red channel, and the odd least significant bytes will form the
        // blue channel.
        int o = i * 2;
        uint8_t g1 = raw_depth_data[o + 1];
        uint8_t g2 = raw_depth_data[o + 3];

        uint8_t r1 = raw_depth_data[o + 0];
        uint8_t r2 = raw_depth_data[o + 0];
        uint8_t b1 = raw_depth_data[o + 2];
        uint8_t b2 = raw_depth_data[o + 2];

        o = i * 3;
        depth_data[o + 0] = r1;
        depth_data[o + 1] = g1;
        depth_data[o + 2] = b1;

        depth_data[o + 3] = r2;
        depth_data[o + 4] = g2;
        depth_data[o + 5] = b2;
      }
      depth_queue.Add(std::move(depth_data));
    }

    if (vf.get_data_size() > 0) {
      const uint8_t* raw_color_data =
          static_cast<const uint8_t*>(vf.get_data());
      std::vector<uint8_t> color_data;
      std::copy_n(raw_color_data, vf.get_data_size(),
                  std::back_inserter(color_data));
      color_queue.Add(std::move(color_data));
    }
  };
  depth_queue.Stop();
  color_queue.Stop();

  return 0;
}