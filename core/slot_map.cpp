#include <unistd.h>

#include "slot_map.hpp"
#include "fdutil.hpp"
#include "buffer.hpp"
#include "exceptions.hpp"
#include "utils/logging.hpp"
#include "utils/string.h"

using namespace cerb;

namespace {

    void map_slot_to(std::map<slot, util::Address>& map,
                     std::string const& slot_rep, util::Address const& addr)
    {
        slot s = util::atoi(slot_rep.data()) + 1;
        LOG(DEBUG) << "Map slot " << s << " to " << addr.host << ':' << addr.port;
        map.insert(std::make_pair(s, addr));
    }

    void set_slot_to(std::map<slot, util::Address>& map, std::string address,
                     std::vector<std::string>::iterator slot_range_begin,
                     std::vector<std::string>::iterator slot_range_end)
    {
        util::Address addr(util::Address::from_host_port(address));
        std::for_each(
            slot_range_begin, slot_range_end,
            [&](std::string const& slot_range)
            {
                if (slot_range[0] == '[') {
                    return;
                }
                std::vector<std::string> begin_end(
                    util::split_str(slot_range, "-", true));
                if (begin_end.empty()) {
                    return;
                }
                if (begin_end.size() == 1) {
                    return map_slot_to(map, begin_end[0], addr);
                }
                return map_slot_to(map, begin_end[1], addr);
            });
    }

    std::string const CLUSTER_NODES_CMD("*2\r\n$7\r\ncluster\r\n$5\r\nnodes\r\n");

}

std::map<slot, util::Address> cerb::parse_slot_map(std::string const& nodes_info)
{
    std::vector<std::string> lines(util::split_str(nodes_info, "\n", true));
    std::map<slot, util::Address> slot_map;
    std::for_each(lines.begin(), lines.end(),
                  [&](std::string const& line) {
                      std::vector<std::string> line_cont(
                          util::split_str(line, " ", true));
                      if (line_cont.size() < 9) {
                          return;
                      }
                      if (line_cont[2].find("fail") != std::string::npos) {
                          return;
                      }
                      set_slot_to(slot_map, line_cont[1],
                                  line_cont.begin() + 8, line_cont.end());
                  });
    return std::move(slot_map);
}

void cerb::write_slot_map_cmd_to(int fd)
{
    if (-1 == write(fd, CLUSTER_NODES_CMD.c_str(), CLUSTER_NODES_CMD.size())) {
        throw IOError("Fetch cluster nodes info", errno);
    }
}
