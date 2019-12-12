/*
 *
 * (C) 2013-19 - ntop.org
 *
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 */

#include "ntop_includes.h"

/* *************************************** */

PartializableFlowTrafficStats::PartializableFlowTrafficStats() {
  ndpiDetectedProtocol = Flow::ndpiUnknownProtocol;
  cli2srv_packets = srv2cli_packets = 0;
  cli2srv_bytes = srv2cli_bytes = 0;
  cli2srv_goodput_bytes = srv2cli_goodput_bytes = 0;

  memset(&cli2srv_tcp_stats, 0, sizeof(cli2srv_tcp_stats));
  memset(&srv2cli_tcp_stats, 0, sizeof(srv2cli_tcp_stats));

  memset(&protos, 0, sizeof(protos));
}

/* *************************************** */

PartializableFlowTrafficStats::PartializableFlowTrafficStats(const PartializableFlowTrafficStats &fts) {
  memcpy(&ndpiDetectedProtocol, &fts.ndpiDetectedProtocol, sizeof(ndpiDetectedProtocol));
  cli2srv_packets = fts.cli2srv_packets;
  srv2cli_packets = fts.srv2cli_packets;
  cli2srv_bytes = fts.cli2srv_bytes;
  srv2cli_bytes = fts.srv2cli_bytes;
  cli2srv_goodput_bytes = fts.cli2srv_goodput_bytes;
  srv2cli_goodput_bytes = fts.srv2cli_goodput_bytes;

  memcpy(&cli2srv_tcp_stats, &fts.cli2srv_tcp_stats, sizeof(cli2srv_tcp_stats));
  memcpy(&srv2cli_tcp_stats, &fts.srv2cli_tcp_stats, sizeof(srv2cli_tcp_stats));

  memcpy(&protos, &fts.protos, sizeof(protos));
}
/* *************************************** */

PartializableFlowTrafficStats PartializableFlowTrafficStats::operator-(const PartializableFlowTrafficStats &fts) {
  PartializableFlowTrafficStats cur(*this);

  cur.cli2srv_packets -= fts.cli2srv_packets;
  cur.cli2srv_bytes -= fts.cli2srv_bytes;
  cur.cli2srv_goodput_bytes -= fts.cli2srv_goodput_bytes;
  cur.srv2cli_packets -= fts.srv2cli_packets;
  cur.srv2cli_bytes -= fts.srv2cli_bytes;
  cur.srv2cli_goodput_bytes -= fts.srv2cli_goodput_bytes;

  cur.cli2srv_tcp_stats.pktRetr -= fts.cli2srv_tcp_stats.pktRetr;
  cur.cli2srv_tcp_stats.pktOOO -= fts.cli2srv_tcp_stats.pktOOO;
  cur.cli2srv_tcp_stats.pktLost -= fts.cli2srv_tcp_stats.pktLost;
  cur.cli2srv_tcp_stats.pktKeepAlive -= fts.cli2srv_tcp_stats.pktKeepAlive;
  cur.srv2cli_tcp_stats.pktRetr -= fts.srv2cli_tcp_stats.pktRetr;
  cur.srv2cli_tcp_stats.pktOOO -= fts.srv2cli_tcp_stats.pktOOO;
  cur.srv2cli_tcp_stats.pktLost -= fts.srv2cli_tcp_stats.pktLost;
  cur.srv2cli_tcp_stats.pktKeepAlive -= fts.srv2cli_tcp_stats.pktKeepAlive;

  switch(ndpi_get_lower_proto(ndpiDetectedProtocol)) {
  case NDPI_PROTOCOL_HTTP:
    cur.protos.http.num_get   -= fts.protos.http.num_get;
    cur.protos.http.num_post  -= fts.protos.http.num_post;
    cur.protos.http.num_put   -= fts.protos.http.num_put;
    cur.protos.http.num_other -= fts.protos.http.num_other;
    cur.protos.http.num_1xx   -= fts.protos.http.num_1xx;
    cur.protos.http.num_2xx   -= fts.protos.http.num_2xx;
    cur.protos.http.num_3xx   -= fts.protos.http.num_3xx;
    cur.protos.http.num_4xx   -= fts.protos.http.num_4xx;
    cur.protos.http.num_5xx   -= fts.protos.http.num_5xx;
    break;
  default:
    break;
  }

  return cur;
}

/* *************************************** */

PartializableFlowTrafficStats::~PartializableFlowTrafficStats() {
}

/* *************************************** */

void PartializableFlowTrafficStats::setDetectedProtocol(const ndpi_protocol *ndpi_detected_protocol) {
  memcpy(&ndpiDetectedProtocol, ndpi_detected_protocol, sizeof(ndpiDetectedProtocol));
}

/* *************************************** */

void PartializableFlowTrafficStats::incTcpStats(bool cli2srv_direction, u_int retr, u_int ooo, u_int lost, u_int keepalive) {
  FlowTCPPacketStats * cur_stats;

  if(cli2srv_direction)
    cur_stats = &cli2srv_tcp_stats;
  else
    cur_stats = &srv2cli_tcp_stats;

  cur_stats->pktKeepAlive += keepalive;
  cur_stats->pktRetr += retr;
  cur_stats->pktOOO += ooo;
  cur_stats->pktLost += lost;
}

/* *************************************** */

void PartializableFlowTrafficStats::incStats(bool cli2srv_direction, u_int num_pkts, u_int pkt_len, u_int payload_len) {
  if(cli2srv_direction)
    cli2srv_packets += num_pkts, cli2srv_bytes += pkt_len, cli2srv_goodput_bytes += payload_len;
  else
    srv2cli_packets += num_pkts, srv2cli_bytes += pkt_len, srv2cli_goodput_bytes += payload_len;
}

/* *************************************** */

void PartializableFlowTrafficStats::setStats(bool cli2srv_direction, u_int num_pkts, u_int pkt_len, u_int payload_len) {
  if(cli2srv_direction)
    cli2srv_packets = num_pkts, cli2srv_bytes = pkt_len, cli2srv_goodput_bytes = payload_len;
  else
    srv2cli_packets = num_pkts, srv2cli_bytes = pkt_len, srv2cli_goodput_bytes = payload_len;
}

/* *************************************** */

void PartializableFlowTrafficStats::get_partial(PartializableFlowTrafficStats **dst, PartializableFlowTrafficStats *fts) const {
  /* Set temp to the current value */
  PartializableFlowTrafficStats tmp(*this); 

  /* Compute the differences between the snapshot tmp and the values found in dst, and put them in the argument fts */
  *fts = tmp - **dst;

  /* Finally, update dst with the values snapshotted in tmp.
     Use the copy constructor to snapshot the value of tmp to dst
  */
  **dst = tmp;
}

/* *************************************** */

u_int16_t PartializableFlowTrafficStats::get_num_http_requests() const {
  return protos.http.num_get + protos.http.num_post + protos.http.num_head + protos.http.num_put + protos.http.num_other;
}