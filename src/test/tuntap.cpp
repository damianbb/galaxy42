#include <array>
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "../tuntap/linux/c_tuntap_linux_obj.hpp"

using testing::Return;
using testing::_;
using testing::Invoke;

TEST(tuntap, bad_open) {
	EXPECT_ANY_THROW(c_tuntap_linux_obj tuntap);
}

TEST(tuntap, send_to_tun) {
	c_tuntap_linux_obj tuntap;
	// normal write to tun
	EXPECT_CALL(tuntap.m_tun_stream, write_some(_)).WillRepeatedly(testing::Invoke(
		[](const boost::asio::const_buffers_1 &buffers) {
			return boost::asio::buffer_size(buffers);
	}));

	std::array<unsigned char, 3> buff1;
	std::array<int, 5> buff2;
	std::array<long int, 8> buff3;
	EXPECT_EQ(tuntap.send_to_tun(buff1.data(), buff1.size()), buff1.size());
	EXPECT_EQ(tuntap.send_to_tun(reinterpret_cast<unsigned char *>(buff2.data()), buff2.size() * sizeof(decltype(buff2)::value_type)),
	          buff2.size() * sizeof(decltype(buff2)::value_type));
	EXPECT_EQ(tuntap.send_to_tun(reinterpret_cast<unsigned char *>(buff3.data()), buff3.size() * sizeof(decltype(buff3)::value_type)),
	          buff3.size() * sizeof(decltype(buff3)::value_type));

}

TEST(tuntap, send_to_tun_with_error) {
	c_tuntap_linux_obj tuntap;
	EXPECT_CALL(tuntap.m_tun_stream, write_some(_)).WillRepeatedly(testing::Invoke(
		[](const boost::asio::const_buffers_1 &buffers) -> size_t {
			// write_some can throw
			// http://www.boost.org/doc/libs/1_61_0/doc/html/boost_asio/reference/posix__basic_stream_descriptor/write_some/overload1.html
			_UNUSED(buffers);
			throw boost::system::system_error(boost::asio::error::eof);
	}));

	std::array<unsigned char, 3> buff1;
	std::array<int, 5> buff2;
	std::array<long int, 8> buff3;
	EXPECT_NO_THROW(tuntap.send_to_tun(buff1.data(), buff1.size()));
	EXPECT_EQ(tuntap.send_to_tun(buff1.data(), buff1.size()), 0U);

	EXPECT_NO_THROW(tuntap.send_to_tun(reinterpret_cast<unsigned char *>(buff2.data()), buff2.size() * sizeof(decltype(buff2)::value_type)));
	EXPECT_EQ(tuntap.send_to_tun(reinterpret_cast<unsigned char *>(buff2.data()), buff2.size() * sizeof(decltype(buff2)::value_type)), 0U);

	EXPECT_NO_THROW(tuntap.send_to_tun(reinterpret_cast<unsigned char *>(buff3.data()), buff3.size() * sizeof(decltype(buff3)::value_type)));
	EXPECT_EQ(tuntap.send_to_tun(reinterpret_cast<unsigned char *>(buff3.data()), buff3.size() * sizeof(decltype(buff3)::value_type)), 0U);
}

static std::array<unsigned char, 103> get_full_packet() {
	std::array<unsigned char, 103> pack;
	pack[0]   = 0x0d; pack[1]   = 0x19; pack[2]   = 0x17; pack[3]   = 0x00; pack[4]   = 0x40; pack[5]   = 0x3a; pack[6]   = 0x40; pack[7]   = 0xfc;
	pack[8]   = 0x72; pack[9]   = 0xaa; pack[10]  = 0x65; pack[11]  = 0xc5; pack[12]  = 0xc2; pack[13]  = 0x4a; pack[14]  = 0x2d; pack[15]  = 0x05;
	pack[16]  = 0x4e; pack[17]  = 0x79; pack[18]  = 0x47; pack[19]  = 0xb6; pack[20]  = 0x71; pack[21]  = 0xe0; pack[22]  = 0x0c; pack[23]  = 0xfc;
	pack[24]  = 0x42; pack[25]  = 0x00; pack[26]  = 0x00; pack[27]  = 0x00; pack[28]  = 0x00; pack[29]  = 0x00; pack[30]  = 0x00; pack[31]  = 0x00;
	pack[32]  = 0x00; pack[33]  = 0x00; pack[34]  = 0x00; pack[35]  = 0x00; pack[36]  = 0x00; pack[37]  = 0x00; pack[38]  = 0x01; pack[39]  = 0x80;
	pack[40]  = 0x00; pack[41]  = 0x9f; pack[42]  = 0xce; pack[43]  = 0x6a; pack[44]  = 0x78; pack[45]  = 0x00; pack[46]  = 0x01; pack[47]  = 0xbe;
	pack[48]  = 0xd1; pack[49]  = 0x22; pack[50]  = 0x59; pack[51]  = 0x00; pack[52]  = 0x00; pack[53]  = 0x00; pack[54]  = 0x00; pack[55]  = 0x05;
	pack[56]  = 0x1f; pack[57]  = 0x08; pack[58]  = 0x00; pack[59]  = 0x00; pack[60]  = 0x00; pack[61]  = 0x00; pack[62]  = 0x00; pack[63]  = 0x10;
	pack[64]  = 0x11; pack[65]  = 0x12; pack[66]  = 0x13; pack[67]  = 0x14; pack[68]  = 0x15; pack[69]  = 0x16; pack[70]  = 0x17; pack[71]  = 0x18;
	pack[72]  = 0x19; pack[73]  = 0x1a; pack[74]  = 0x1b; pack[75]  = 0x1c; pack[76]  = 0x1d; pack[77]  = 0x1e; pack[78]  = 0x1f; pack[79]  = 0x20;
	pack[80]  = 0x21; pack[81]  = 0x22; pack[82]  = 0x23; pack[83]  = 0x24; pack[84]  = 0x25; pack[85]  = 0x26; pack[86]  = 0x27; pack[87]  = 0x28;
	pack[88]  = 0x29; pack[89]  = 0x2a; pack[90]  = 0x2b; pack[91]  = 0x2c; pack[92]  = 0x2d; pack[93]  = 0x2e; pack[94]  = 0x2f; pack[95]  = 0x30;
	pack[96]  = 0x31; pack[97]  = 0x32; pack[98]  = 0x33; pack[99]  = 0x34; pack[100] = 0x35; pack[101] = 0x36; pack[102] = 0x37;
	return pack;
}

static std::tuple<
	std::array<unsigned char, IPV6_LEN>, // src address
	std::array<unsigned char, IPV6_LEN>, // dst address
	std::array<unsigned char, 72> // ipv6 without src and dst
> get_separated_packet() {
	std::array<unsigned char, IPV6_LEN> src {{0xfc, 0x72, 0xaa, 0x65, 0xc5, 0xc2, 0x4a, 0x2d, 0x05, 0x4e, 0x79, 0x47, 0xb6, 0x71, 0xe0, 0x0c}};
	std::array<unsigned char, IPV6_LEN> dst {{0xfc, 0x42, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}};
	std::array<unsigned char, 72> data{{0x00}};
	return std::make_tuple(src, dst, data);
}

TEST(tuntap, send_to_tun_seperated) {
	c_tuntap_linux_obj tuntap;
	std::array<unsigned char, IPV6_LEN> src, dst;
	std::array<unsigned char, 72> packet;
	std::tie(src, dst, packet) = get_separated_packet();

	EXPECT_CALL(tuntap.m_tun_stream, write_some(_, _)).WillOnce(testing::Invoke(
		[](std::array<boost::asio::const_buffer, 4> &buf ,boost::system::error_code &err) -> size_t {
			_UNUSED(err);
			return boost::asio::buffer_size(buf);
		}
	));
	EXPECT_EQ (
		tuntap.send_to_tun_separated_addresses(packet.data(), packet.size(), src, dst),
		src.size() + dst.size() + packet.size()
	);

	// write with error
	EXPECT_CALL(tuntap.m_tun_stream, write_some(_, _)).WillOnce(testing::Invoke(
		[](std::array<boost::asio::const_buffer, 4> &buf ,boost::system::error_code &err) -> size_t {
			_UNUSED(buf);
			err = boost::asio::error::eof; // some error
			return 0;
		}
	));
	EXPECT_EQ(
		tuntap.send_to_tun_separated_addresses(packet.data(), packet.size(), src, dst),
		0U
	);
}
