#include <rosbag/bag.h>
#include <rosbag/view.h>
#include <rosbag/query.h>
#include "progressbar.hpp"
#include <ctime>
#include <iostream>


// -o, --output Output bag file
// -c, --compression Compression format: none, lw4 or bz2 (default lw4)
// -p, --progress Display progress
// -s, --start_time Skip messages earlier than start time
// -e, --end_time Skip messages later than end time
// -h, --help Display this message

std::map<std::string, bool> exclude_map;

bool checkExclude(const rosbag::ConnectionInfo* info)
{
  return !exclude_map[info->topic];
}

int main(int argc, char *argv[])
{
//################ Defaults ###################

  std::string out_name = "out.bag";
  std::string compression_string = "lz4";
  bool progress = false;
  ros::Time start = ros::TIME_MIN; 
  ros::Time end = ros::TIME_MAX;
  
//#############################################

//################ Read in args ###################

  std::vector<std::string> arguments(argv+1,argv+argc);

  if (arguments.empty()) 
  {
    std::cout << "[ERROR]: No args, -h for help"<< std::endl;
    return -1;
  }
  
  std::vector<std::string> bagfile_names;
  std::vector<std::string> include_topics;
  std::vector<std::string> exclude_topics;

  for (auto arg = arguments.begin(); arg != arguments.end();arg++) 
  {
    if (*arg == "-h")
    {
      std::cout << "-o, --output Output bag file name" << std::endl;
      std::cout << "-c, --compression Compression format: none, lz4 or bz2 (default lz4)" << std::endl;
      std::cout << "-p, --progress Display progress" << std::endl;
      std::cout << "-s, --start_time Skip messages earlier than start time (Y-m-d-H:M:S) (local time)" << std::endl;
      std::cout << "-e, --end_time Skip messages later than end time (Y-m-d-H:M:S) (local time)" << std::endl;
       std::cout << "-x, --exclude Exclude a topic, may be repeated" << std::endl;
      std::cout << "-i, --include Ixclude a topic, may be repeated" << std::endl;
      std::cout << "-t, --topic Topic filtering config file" << std::endl;
      std::cout << "-h, --help Display this message" << std::endl;
      return -1;
    }
    else if (*arg == "-o")
    {
      arg++;
      out_name = *arg;
    }
    else if (*arg == "-c")
    {
      arg++;
      compression_string = *arg;
    }
    else if (*arg == "-p")
    {
      progress = true;
    }
    else if (*arg == "-s")
    {
      arg++;
      struct tm tm;
      std::string c = *arg;
      const char * strpt = c.c_str();
      strptime(strpt, "%Y-%m-%d-%H:%M:%S",&tm);
      time_t start_time = mktime(&tm);
      start = ros::Time(start_time);
      if (progress == true)
      {
        std::cout << "[INFO] Merging with start time: " << start_time << std::endl;
      }
    }
    else if (*arg == "-e")
    {
      arg++;
      struct tm tm;
      std::string c = *arg;
      const char * strpt = c.c_str();
      strptime(strpt, "%Y-%m-%d-%H:%M:%S",&tm);
      time_t end_time = mktime(&tm);
      end = ros::Time(end_time);
      if (progress == true)
      {
        std::cout << "[INFO] Merging with end time: " << end_time << std::endl;
      }
    }
     else if (*arg == "-i")
    {
      arg++;
      if (exclude_topics.empty())
      {
        include_topics.push_back(*arg);
        std::cout << "[INFO] Including topic: " << *arg << std::endl;
      }
      else
      {
        std::cout << "conflicting include and exclude args please use a config file to do complex topic filtering" << std::endl;
      }
    }
     else if (*arg == "-x")
    {
      arg++;
      if (include_topics.empty())
      {
        exclude_map[*arg] = true;
        std::cout << "[INFO] excluding topic: " << *arg << std::endl;
      }
      else
      {
        std::cout << "conflicting include and exclude args please use a config file to do complex topic filtering" << std::endl;
      }
    }

//################ Read in all non-args as paths to bags ###################

    else
    {
      std::ifstream test(*arg); 
      if (!test)
      {
        std::cout << "[ERROR] File or path doesn't exist: " << *arg << std::endl;
        return -1;
      }
      else
      {
        bagfile_names.push_back(*arg);
      }
    }
  }

//################ Adjust settings based on args / defaults ###################

  rosbag::CompressionType compression;
  if (compression_string == "lz4")
  {
    compression = rosbag::CompressionType::LZ4;
    if ( progress == true)
    {
      std::cout << "[INFO] Compression set to LZ4" << std::endl;
    }
  }
  if (compression_string == "bz2")
  {
    compression = rosbag::CompressionType::BZ2;
    if ( progress == true)
    {
      std::cout << "[INFO] Compression set to BZ2" << std::endl;
    }
  }
  if (compression_string == "none")
  {
    compression = rosbag::CompressionType::Uncompressed;
    if ( progress == true)
    {
      std::cout << "[INFO] Compression set to none" << std::endl;
    }
  }

//################ Add relevant rosbags to view ###################

  rosbag::Bag outbag;
  outbag.open(out_name, rosbag::bagmode::Write);
  outbag.setCompression(compression);
  int size = 0;
  rosbag::View merged_view(true);
  std::vector<std::shared_ptr<rosbag::Bag> > bags;
  for (auto arg = bagfile_names.begin(); arg != bagfile_names.end();arg++)
  { 
    if (progress == true)
    {
    std::cout << "[INFO] Opening bag " << *arg <<std::endl;
    }
    if (!include_topics.empty())
    {
      std::vector<std::string> topics_filt;
      topics_filt = include_topics;
      bags.push_back(std::shared_ptr<rosbag::Bag>(new rosbag::Bag));
      bags.back()->open(*arg, rosbag::bagmode::Read);
      merged_view.addQuery(*bags.back(), rosbag::TopicQuery(topics_filt),start,end);
    }
    else 
    {
      bags.push_back(std::shared_ptr<rosbag::Bag>(new rosbag::Bag));
      bags.back()->open(*arg, rosbag::bagmode::Read);
      merged_view.addQuery(*bags.back(),&checkExclude,start,end);
    }
  }
  
//################ Review all topics in view ###################

  size = merged_view.size();
  std::vector<const rosbag::ConnectionInfo *> connection_infos = merged_view.getConnections();
  std::set<std::string> topics;
  for (const rosbag::ConnectionInfo* info: connection_infos) 
  {
  topics.insert(info->topic);
  }

  if (progress == true)
  {
    std::cout << "Merging topics: " << std::endl;
    for (auto it: topics)
    {
      std::cout << ' ' << it << std::endl;
    }
  }
 
//################ Write view to output bag ###################

  progressbar bar(size);
  for (rosbag::MessageInstance const m: merged_view)
  {
    if (progress == true)
    {
      bar.update();
    }
      outbag.write(m.getTopic(), m.getTime(), m, m.getConnectionHeader());
  }
  std::cout << std::endl;
  return 0;
}
