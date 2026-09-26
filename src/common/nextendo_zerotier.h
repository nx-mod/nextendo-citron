// SPDX-FileCopyrightText: 2026 nx-mod
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

// Address handling for the "ZeroTier" Nextendo option.
//
// With the option on, everything Nextendo does goes to one address the user types in: the game
// hostnames (sfdnsres), the NAT check, and the account API. The account API carries the sign-in
// token, so the address is only accepted when it is a private IPv4 literal -- the ZeroTier /
// LAN case. A public or hostname value can never receive the token this way; it falls back to
// the canonical server instead.
//
// Header-only and free of Citron types so it can be tested on its own.

#include <cstdint>
#include <optional>
#include <string>
#include <string_view>

namespace Common::NextendoZeroTier {

constexpr int DefaultAccountPort = 8080;

struct Endpoint {
    std::string host; // dotted-quad IPv4
    int port = 0;     // 0 when the address carried none
};

namespace detail {

inline std::string_view Trim(std::string_view s) {
    while (!s.empty() && (s.front() == ' ' || s.front() == '\t')) {
        s.remove_prefix(1);
    }
    while (!s.empty() && (s.back() == ' ' || s.back() == '\t' || s.back() == '/')) {
        s.remove_suffix(1);
    }
    return s;
}

// Parses "a.b.c.d" strictly: four decimal parts, each 0-255, no signs, no leading junk.
inline std::optional<std::uint32_t> ParseIpv4(std::string_view s) {
    std::uint32_t value = 0;
    int parts = 0;
    std::size_t i = 0;
    while (i <= s.size()) {
        std::size_t j = i;
        int n = 0;
        int digits = 0;
        while (j < s.size() && s[j] >= '0' && s[j] <= '9') {
            n = n * 10 + (s[j] - '0');
            if (++digits > 3 || n > 255) {
                return std::nullopt;
            }
            ++j;
        }
        if (digits == 0) {
            return std::nullopt;
        }
        value = (value << 8) | static_cast<std::uint32_t>(n);
        ++parts;
        if (j == s.size()) {
            break;
        }
        if (s[j] != '.') {
            return std::nullopt;
        }
        i = j + 1;
    }
    return parts == 4 ? std::optional<std::uint32_t>{value} : std::nullopt;
}

} // namespace detail

// RFC 1918 (10/8, 172.16/12, 192.168/16) and carrier-grade NAT (100.64/10).
inline bool IsPrivateIpv4(std::string_view host) {
    const auto ip = detail::ParseIpv4(host);
    if (!ip) {
        return false;
    }
    const std::uint32_t v = *ip;
    return (v >> 24) == 10 || (v >> 20) == 0xAC1 /* 172.16/12 */ || (v >> 16) == 0xC0A8 ||
           (v >> 22) == 0x191 /* 100.64/10 */;
}

// Accepts "10.1.2.3", "10.1.2.3:8080", "http://10.1.2.3", "http://10.1.2.3:8080/".
// Rejects anything that is not a private IPv4 literal, and a bad or out-of-range port.
inline std::optional<Endpoint> ParseAddress(std::string_view address) {
    std::string_view s = detail::Trim(address);
    if (const auto scheme = s.find("://"); scheme != std::string_view::npos) {
        if (s.substr(0, scheme) != "http") {
            return std::nullopt;
        }
        s.remove_prefix(scheme + 3);
    }
    if (s.empty() || s.find('/') != std::string_view::npos) {
        return std::nullopt;
    }

    Endpoint out;
    std::string_view host = s;
    if (const auto colon = s.find(':'); colon != std::string_view::npos) {
        host = s.substr(0, colon);
        const std::string_view port = s.substr(colon + 1);
        if (port.empty() || port.size() > 5) {
            return std::nullopt;
        }
        int p = 0;
        for (const char c : port) {
            if (c < '0' || c > '9') {
                return std::nullopt;
            }
            p = p * 10 + (c - '0');
        }
        if (p < 1 || p > 65535) {
            return std::nullopt;
        }
        out.port = p;
    }
    if (!IsPrivateIpv4(host)) {
        return std::nullopt;
    }
    out.host = std::string{host};
    return out;
}

// The bare host to hand to the DNS redirect ("10.1.2.3:8080" -> "10.1.2.3"), or nullopt.
inline std::optional<std::string> HostOnly(std::string_view address) {
    if (const auto ep = ParseAddress(address)) {
        return ep->host;
    }
    return std::nullopt;
}

// "http://host:port" for the account API. Plain http is fine here: the address is private and
// the traffic rides the ZeroTier tunnel.
inline std::optional<std::string> AccountBaseUrl(std::string_view address,
                                                 int default_port = DefaultAccountPort) {
    const auto ep = ParseAddress(address);
    if (!ep) {
        return std::nullopt;
    }
    return "http://" + ep->host + ":" + std::to_string(ep->port != 0 ? ep->port : default_port);
}

} // namespace Common::NextendoZeroTier
