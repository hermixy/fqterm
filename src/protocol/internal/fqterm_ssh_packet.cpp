/***************************************************************************
 *   fqterm, a terminal emulator for both BBS and *nix.                    *
 *   Copyright (C) 2008 fqterm development group.                          *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA.               *
 ***************************************************************************/

#include "fqterm_trace.h"
#include "fqterm_ssh_buffer.h"
#include "fqterm_ssh_packet.h"
#include "fqterm_ssh_const.h"

#include "fqterm_serialization.h"
#include "crc32.h"

namespace FQTerm {

//==============================================================================
//FQTermSSHPacketSender
//==============================================================================

FQTermSSHPacketSender::FQTermSSHPacketSender() {
  buffer_ = new FQTermSSHBuffer(1024);
  output_buffer_ = new FQTermSSHBuffer(1024);

  is_encrypt_ = false;
  cipher_type_ = SSH_CIPHER_NONE;
  cipher = NULL;

  is_mac_ = false;
  mac = NULL;

  is_compressed_ = false;

  sequence_no_ = 0;
}

FQTermSSHPacketSender::~FQTermSSHPacketSender() {
	delete buffer_;
	delete output_buffer_;
	if (cipher)
		cipher->cleanup(cipher);
        if (mac)
		mac->cleanup(mac);
}

void FQTermSSHPacketSender::putRawData(const char *data, int len) {
  buffer_->putRawData(data, len);
}

void FQTermSSHPacketSender::putByte(int data) {
  buffer_->putByte(data);
}

void FQTermSSHPacketSender::putInt(u_int data) {
  buffer_->putInt(data);
}

void FQTermSSHPacketSender::putString(const char *string, int len) {
  buffer_->putString(string, len);
}

void FQTermSSHPacketSender::putBN(BIGNUM *bn) {
  buffer_->putSSH1BN(bn);
}

void FQTermSSHPacketSender::putBN2(BIGNUM *bn) {
  buffer_->putSSH2BN(bn);
}

void FQTermSSHPacketSender::startPacket(int pkt_type) {
  buffer_->clear();
  buffer_->putByte(pkt_type);
}

void FQTermSSHPacketSender::write() {
  makePacket();
  emit dataToWrite();
}

void FQTermSSHPacketSender::startEncryption(const u_char *key, const u_char *IV) {
	is_encrypt_ = true;

	if (cipher!=NULL) {
		memcpy(cipher->IV, IV, cipher->IVSize);
		memcpy(cipher->key, key, cipher->keySize);
		cipher->init(cipher);
	}
}

void FQTermSSHPacketSender::resetEncryption() {
  is_encrypt_ = false;
}

void FQTermSSHPacketSender::startMac(const u_char *key) {
  is_mac_ = true;
  memcpy(mac->key, key, mac->keySize);
}

void FQTermSSHPacketSender::resetMac() {
  is_mac_ = false;
}

//==============================================================================
//FQTermSSHPacketReceiver
//==============================================================================

FQTermSSHPacketReceiver::FQTermSSHPacketReceiver() {
  buffer_ = new FQTermSSHBuffer(1024);

  is_decrypt_ = false;
  cipher_type_ = SSH_CIPHER_NONE;
  cipher = NULL;

  is_mac_ = false;
  mac = NULL;

  is_compressed_ = false;

  sequence_no_ = 0;
}

FQTermSSHPacketReceiver::~FQTermSSHPacketReceiver()
{
	delete buffer_;
	if (cipher)
		cipher->cleanup(cipher);
        if (mac)
		mac->cleanup(mac);
}

void FQTermSSHPacketReceiver::getRawData(char *data, int length) {
  buffer_->getRawData(data, length);
}

u_char FQTermSSHPacketReceiver::getByte() {
  return buffer_->getByte();
}

u_int FQTermSSHPacketReceiver::getInt() {
  return buffer_->getInt();
}

void *FQTermSSHPacketReceiver::getString(int *length) {
  return buffer_->getString(length);
}

void FQTermSSHPacketReceiver::getBN(BIGNUM *bignum) {
  buffer_->getSSH1BN(bignum);
}

void FQTermSSHPacketReceiver::getBN2(BIGNUM *bignum) {
  buffer_->getSSH2BN(bignum);
}

void FQTermSSHPacketReceiver::consume(int len) {
  buffer_->consume(len);
}

void FQTermSSHPacketReceiver::startEncryption(const u_char *key, const u_char *IV) {
	is_decrypt_ = true;

	if (cipher!=NULL) {
		memcpy(cipher->IV, IV, cipher->IVSize);
		memcpy(cipher->key, key, cipher->keySize);
		cipher->init(cipher);
	}
}

void FQTermSSHPacketReceiver::resetEncryption() {
  is_decrypt_ = false;
}

void FQTermSSHPacketReceiver::startMac(const u_char *key) {
  is_mac_ = true;
  memcpy(mac->key, key, mac->keySize);
}

void FQTermSSHPacketReceiver::resetMac() {
  is_mac_ = false;
}

}  // namespace FQTerm

#include "fqterm_ssh_packet.moc"
