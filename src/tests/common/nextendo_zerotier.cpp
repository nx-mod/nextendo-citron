// SPDX-FileCopyrightText: 2026 nx-mod
// SPDX-License-Identifier: GPL-3.0-or-later

#include <catch2/catch_test_macros.hpp>

#include "common/nextendo_zerotier.h"

using namespace Common::NextendoZeroTier;

TEST_CASE("NextendoZeroTier: private IPv4 ranges", "[common]") {
    REQUIRE(IsPrivateIpv4("10.214.216.58"));
    REQUIRE(IsPrivateIpv4("10.0.0.1"));
    REQUIRE(IsPrivateIpv4("172.16.0.1"));
    REQUIRE(IsPrivateIpv4("172.31.255.255"));
    REQUIRE(IsPrivateIpv4("192.168.1.1"));
    REQUIRE(IsPrivateIpv4("100.64.0.1"));
    REQUIRE(IsPrivateIpv4("100.127.255.255"));

    // Just outside each range.
    REQUIRE_FALSE(IsPrivateIpv4("172.15.255.255"));
    REQUIRE_FALSE(IsPrivateIpv4("172.32.0.1"));
    REQUIRE_FALSE(IsPrivateIpv4("100.63.255.255"));
    REQUIRE_FALSE(IsPrivateIpv4("100.128.0.1"));
    REQUIRE_FALSE(IsPrivateIpv4("192.169.0.1"));
    REQUIRE_FALSE(IsPrivateIpv4("11.0.0.1"));
    // Public addresses, including the real Nextendo servers.
    REQUIRE_FALSE(IsPrivateIpv4("51.178.29.194"));
    REQUIRE_FALSE(IsPrivateIpv4("164.132.111.120"));
    REQUIRE_FALSE(IsPrivateIpv4("8.8.8.8"));
}

TEST_CASE("NextendoZeroTier: malformed addresses are not private", "[common]") {
    REQUIRE_FALSE(IsPrivateIpv4(""));
    REQUIRE_FALSE(IsPrivateIpv4("10.0.0"));
    REQUIRE_FALSE(IsPrivateIpv4("10.0.0.1.5"));
    REQUIRE_FALSE(IsPrivateIpv4("10.0.0.256"));
    REQUIRE_FALSE(IsPrivateIpv4("10.0..1"));
    REQUIRE_FALSE(IsPrivateIpv4("10.0.0.1 "));
    REQUIRE_FALSE(IsPrivateIpv4("+10.0.0.1"));
    REQUIRE_FALSE(IsPrivateIpv4("nextendo.network"));
    REQUIRE_FALSE(IsPrivateIpv4("localhost"));
    REQUIRE_FALSE(IsPrivateIpv4("10.0.0.1000"));
}

TEST_CASE("NextendoZeroTier: ParseAddress forms", "[common]") {
    auto ep = ParseAddress("10.214.216.58");
    REQUIRE(ep.has_value());
    REQUIRE(ep->host == "10.214.216.58");
    REQUIRE(ep->port == 0);

    ep = ParseAddress("  10.214.216.58:8085  ");
    REQUIRE(ep.has_value());
    REQUIRE(ep->port == 8085);

    ep = ParseAddress("http://10.214.216.58:8080/");
    REQUIRE(ep.has_value());
    REQUIRE(ep->host == "10.214.216.58");
    REQUIRE(ep->port == 8080);

    // https is refused: the address is private and plain http is the supported transport, and an
    // https URL to a private literal could not present a valid certificate anyway.
    REQUIRE_FALSE(ParseAddress("https://10.214.216.58").has_value());
    REQUIRE_FALSE(ParseAddress("ftp://10.214.216.58").has_value());
    REQUIRE_FALSE(ParseAddress("10.214.216.58/path").has_value());
    REQUIRE_FALSE(ParseAddress("10.214.216.58:").has_value());
    REQUIRE_FALSE(ParseAddress("10.214.216.58:0").has_value());
    REQUIRE_FALSE(ParseAddress("10.214.216.58:65536").has_value());
    REQUIRE_FALSE(ParseAddress("10.214.216.58:80a").has_value());
    REQUIRE_FALSE(ParseAddress("").has_value());
}

TEST_CASE("NextendoZeroTier: the sign-in token can only go to a private address", "[common]") {
    REQUIRE(AccountBaseUrl("10.214.216.58") == "http://10.214.216.58:8080");
    REQUIRE(AccountBaseUrl("10.214.216.58:9000") == "http://10.214.216.58:9000");
    REQUIRE(AccountBaseUrl("http://192.168.1.5") == "http://192.168.1.5:8080");
    REQUIRE(AccountBaseUrl("10.214.216.58", 1234) == "http://10.214.216.58:1234");

    // Anything public or named must be refused so the caller falls back to the canonical server.
    REQUIRE_FALSE(AccountBaseUrl("51.178.29.194").has_value());
    REQUIRE_FALSE(AccountBaseUrl("nextendo.network").has_value());
    REQUIRE_FALSE(AccountBaseUrl("evil.example").has_value());
    REQUIRE_FALSE(AccountBaseUrl("").has_value());
    REQUIRE_FALSE(AccountBaseUrl("127.0.0.1").has_value()); // loopback already has its own path
}

TEST_CASE("NextendoZeroTier: HostOnly strips scheme and port", "[common]") {
    REQUIRE(HostOnly("10.214.216.58") == "10.214.216.58");
    REQUIRE(HostOnly("10.214.216.58:8080") == "10.214.216.58");
    REQUIRE(HostOnly("http://10.214.216.200:1/") == "10.214.216.200");
    REQUIRE_FALSE(HostOnly("8.8.8.8").has_value());
}
