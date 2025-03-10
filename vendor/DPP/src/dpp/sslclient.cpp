/************************************************************************************
 *
 * D++, A Lightweight C++ library for Discord
 *
 * Copyright 2021 Craig Edwards and D++ contributors 
 * (https://github.com/brainboxdotcc/DPP/graphs/contributors)
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 ************************************************************************************/
#include <errno.h>

#ifdef _WIN32
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <io.h>
#pragma comment(lib,"ws2_32")
#else
#include <resolv.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <unistd.h>
#endif

#ifdef OPENSSL_SYS_WIN32
#undef X509_NAME
#undef X509_EXTENSIONS
#undef X509_CERT_PAIR
#undef PKCS7_ISSUER_AND_SERIAL
#undef OCSP_REQUEST
#undef OCSP_RESPONSE
#endif

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <fcntl.h>
#include <signal.h>
#include <string.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <exception>
#include <string>
#include <iostream>
#include <fmt/format.h>
#include <dpp/sslclient.h>
#include <dpp/discord.h>
#include <dpp/dispatcher.h>

namespace dpp {

class opensslcontext {
public:
	/** OpenSSL session */
	SSL* ssl;

	/** OpenSSL context */
	SSL_CTX* ctx;
};

/* You'd think that we would get better performance with a bigger buffer, but SSL frames are 16k each.
 * SSL_read in non-blocking mode will only read 16k at a time. There's no point in a bigger buffer as
 * it'd go unused.
 */
#define BUFSIZZ 1024 * 16
const int ERROR_STATUS = -1;

ssl_client::ssl_client(const std::string &_hostname, const std::string &_port) : last_tick(time(NULL)), hostname(_hostname), port(_port), bytes_in(0), bytes_out(0)
{
#ifndef WIN32
        signal(SIGALRM, SIG_IGN);
        signal(SIGHUP, SIG_IGN);
        signal(SIGPIPE, SIG_IGN);
        signal(SIGCHLD, SIG_IGN);
        signal(SIGXFSZ, SIG_IGN);
#endif
	ssl = new opensslcontext();
	Connect();
}

/* SSL Client constructor throws std::runtime_error if it can't connect to the host */
void ssl_client::Connect()
{
	/* Initial connection is done in blocking mode. There is a timeout on it. */
	nonblocking = false;
	const SSL_METHOD *method = TLS_client_method(); /* Create new client-method instance */

	/* Create SSL context */
	ssl->ctx = SSL_CTX_new(method);
	if (ssl->ctx == nullptr)
		throw dpp::exception("Failed to create SSL client context!");

	/* Create SSL session */
	ssl->ssl = SSL_new(ssl->ctx);
	if (ssl->ssl == nullptr)
		throw dpp::exception("SSL_new failed!");

	/* Resolve hostname to IP */
	struct hostent *host;
	if ((host = gethostbyname(hostname.c_str())) == nullptr)
		throw dpp::exception(fmt::format("Couldn't resolve hostname '{}'", hostname));

	struct addrinfo hints = {0}, *addrs;
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	int status = getaddrinfo(hostname.c_str(), port.c_str(), &hints, &addrs);
	if (status != 0)
		throw dpp::exception(fmt::format("getaddrinfo (host={}, port={}): ", hostname, port, gai_strerror(status)));

	/* Attempt each address in turn, if there are multiple IP addresses on the hostname */
	int err;
	for (struct addrinfo *addr = addrs; addr != nullptr; addr = addr->ai_next) {
		sfd = socket(addrs->ai_family, addrs->ai_socktype, addrs->ai_protocol);
		if (sfd == ERROR_STATUS) {
			err = errno;
			continue;
		} else if (connect(sfd, addr->ai_addr, addr->ai_addrlen) == 0) {
			break;
		}
		err = errno;
		shutdown(sfd, 2);
	#ifdef _WIN32
		if (sfd >= 0 && sfd < FD_SETSIZE) {
			closesocket(sfd);
		}
	#else
		::close(sfd);
	#endif
		sfd = ERROR_STATUS;
	}
	freeaddrinfo(addrs);

	/* Check if none of the IPs yielded a valid connection */
	if (sfd == ERROR_STATUS)
		throw dpp::exception(strerror(err));

	/* We're good to go - hand the fd over to openssl */
	SSL_set_fd(ssl->ssl, sfd);

	status = SSL_connect(ssl->ssl);
	if (status != 1) {
		throw dpp::exception("SSL_connect error");
	}

	this->cipher = SSL_get_cipher(ssl->ssl);
}

void ssl_client::write(const std::string &data)
{
	/* If we are in nonblocking mode, append to the buffer,
	 * otherwise just use SSL_write directly. The only time we
	 * use SSL_write directly is during connection before the
	 * ReadLoop is called, which allows for guaranteed simple
	 * lock-step delivery e.g. for HTTP header negotiation
	 */
	if (nonblocking) {
		obuffer += data;
	} else {
		SSL_write(ssl->ssl, data.data(), data.length());
	}
}

void ssl_client::one_second_timer()
{
}

std::string ssl_client::get_cipher() {
	return cipher;
}

void ssl_client::log(dpp::loglevel severity, const std::string &msg) const
{
}

void ssl_client::read_loop()
{
	/* The read loop is non-blocking using select(). This method
	 * cannot read while it is waiting for write, or write while it is
	 * waiting for read. This is a limitation of the openssl libraries,
	 * as SSL is sent and received in low level ~16k frames which must
	 * be synchronised and ordered correctly. Attempting to send while
	 * we need another frame or receive while we are due to send a frame
	 * would cause the protocol to break.
	 */
	int width;
	int r = 0;
	size_t ClientToServerLength = 0, ClientToServerOffset = 0;
	bool read_blocked_on_write =  false, write_blocked_on_read = false,read_blocked = false;
	fd_set readfds, writefds, efds;
	char ClientToServerBuffer[BUFSIZZ], ServerToClientBuffer[BUFSIZZ];
	
	/* Make the socket nonblocking */
#ifdef _WIN32
	u_long mode = 1;
	int result = ioctlsocket(sfd, FIONBIO, &mode);
	if (result != NO_ERROR)
		throw dpp::exception("Can't switch socket to non-blocking mode!");
#else
	int ofcmode;
	ofcmode = fcntl(sfd, F_GETFL, 0);
	ofcmode |= O_NDELAY;
	if (fcntl(sfd, F_SETFL, ofcmode)) {
		throw dpp::exception("Can't switch socket to non-blocking mode!");
	}
#endif
	nonblocking = true;
	width = sfd + 1;
	
	try {
		/* Loop until there is a socket error */
		while(true) {

			if (last_tick != time(NULL)) {
				this->one_second_timer();
				last_tick = time(NULL);
			}

			FD_ZERO(&readfds);
			FD_ZERO(&writefds);
			FD_ZERO(&efds);

			FD_SET(sfd,&readfds);
			FD_SET(sfd,&efds);
			if (custom_readable_fd && custom_readable_fd() >= 0) {
				int cfd = custom_readable_fd();
				FD_SET(cfd, &readfds);
				FD_SET(cfd, &efds);
			}
			if (custom_writeable_fd && custom_writeable_fd() >= 0) {
				int cfd = custom_writeable_fd();
				FD_SET(cfd, &writefds);
			}

			/* If we're waiting for a read on the socket don't try to write to the server */
			if (ClientToServerLength || obuffer.length() || read_blocked_on_write) {
				FD_SET(sfd,&writefds);
			}
				
			timeval ts;
			ts.tv_sec = 0;
			ts.tv_usec = 50000;
			r = select(FD_SETSIZE, &readfds, &writefds, &efds, &ts);
			if (r == 0)
				continue;

			if (custom_writeable_fd && FD_ISSET(custom_writeable_fd(), &writefds)) {
				custom_writeable_ready();
			}
			if (custom_readable_fd && FD_ISSET(custom_readable_fd(), &readfds)) {
				custom_readable_ready();
			}
			if (custom_readable_fd && FD_ISSET(custom_readable_fd(), &efds)) {
			}

			if (FD_ISSET(sfd, &efds)) {
				this->log(dpp::ll_error, fmt::format("Error on SSL connection: {}", strerror(errno)));
				return;
			}

			/* Now check if there's data to read */
			if((FD_ISSET(sfd,&readfds) && !write_blocked_on_read) || (read_blocked_on_write && FD_ISSET(sfd,&writefds))) {
				do {
					read_blocked_on_write = false;
					read_blocked = false;
					
					r = SSL_read(ssl->ssl,ServerToClientBuffer,BUFSIZZ);

					int e = SSL_get_error(ssl->ssl,r);

					switch (e) {
						case SSL_ERROR_NONE:
							/* Data received, add it to the buffer */
							buffer.append(ServerToClientBuffer, r);
							this->handle_buffer(buffer);
							bytes_in += r;
						break;
						case SSL_ERROR_ZERO_RETURN:
							/* End of data */
							SSL_shutdown(ssl->ssl);
							return;
						break;
						case SSL_ERROR_WANT_READ:
							read_blocked = true;
						break;
								
						/* We get a WANT_WRITE if we're trying to rehandshake and we block on a write during that rehandshake.
						* We need to wait on the socket to be writeable but reinitiate the read when it is
						*/
						case SSL_ERROR_WANT_WRITE:
							read_blocked_on_write = true;
						break;
						default:
							return;
						break;
					}

					/* We need a check for read_blocked here because SSL_pending() doesn't work properly during the
					* handshake. This check prevents a busy-wait loop around SSL_read()
					*/
				} while (SSL_pending(ssl->ssl) && !read_blocked);
			}

			/* Check for input on the sendq */
			if (obuffer.length() && ClientToServerLength == 0) {
				memcpy(&ClientToServerBuffer, obuffer.data(), obuffer.length() > BUFSIZZ ? BUFSIZZ : obuffer.length());
				ClientToServerLength = obuffer.length() > BUFSIZZ ? BUFSIZZ : obuffer.length();
				obuffer = obuffer.substr(ClientToServerLength, obuffer.length());
				ClientToServerOffset = 0;
			}

			/* If the socket is writeable... */
			if ((FD_ISSET(sfd,&writefds) && ClientToServerLength) || (write_blocked_on_read && FD_ISSET(sfd,&readfds))) {
				write_blocked_on_read = false;
				/* Try to write */
				r = SSL_write(ssl->ssl, ClientToServerBuffer + ClientToServerOffset, ClientToServerLength);
				
				switch(SSL_get_error(ssl->ssl,r)) {
					/* We wrote something */
					case SSL_ERROR_NONE:
						ClientToServerLength -= r;
						ClientToServerOffset += r;
						bytes_out += r;
					break;
						
					/* We would have blocked */
					case SSL_ERROR_WANT_WRITE:
					break;
			
					/* We get a WANT_READ if we're trying to rehandshake and we block onwrite during the current connection.
					* We need to wait on the socket to be readable but reinitiate our write when it is
					*/
					case SSL_ERROR_WANT_READ:
						write_blocked_on_read = true;
					break;
							
					/* Some other error */
					default:
						return;
					break;
				}
			}
		}
	}
	catch (const std::exception &e) {
		log(ll_warning, fmt::format("Read loop ended: {}", e.what()));
	}
}

uint64_t ssl_client::get_bytes_out()
{
	return bytes_out;
}

uint64_t ssl_client::get_bytes_in()
{
	return bytes_in;
}

bool ssl_client::handle_buffer(std::string &buffer)
{
	return true;
}

void ssl_client::close()
{
	if (ssl->ssl) {
		SSL_free(ssl->ssl);
		ssl->ssl = nullptr;
	}
	shutdown(sfd, 2);
	#ifdef _WIN32
		if (sfd >= 0 && sfd < FD_SETSIZE) {
			closesocket(sfd);
		}
	#else
		::close(sfd);
	#endif
	if (ssl->ctx) {
		SSL_CTX_free(ssl->ctx);
		ssl->ctx = nullptr;
	}
	sfd = -1;
	obuffer.clear();
	buffer.clear();
}

ssl_client::~ssl_client()
{
	this->close();
	delete ssl;
}

};