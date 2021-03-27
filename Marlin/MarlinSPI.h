/**
 * Marlin 3D Printer Firmware
 * Copyright (C) 2016 MarlinFirmware [https://github.com/MarlinFirmware/Marlin]
 *
 * Based on Sprinter and grbl.
 * Copyright (C) 2011 Camiel Gubbels / Erik van der Zalm
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef __MARLIN_SPI_H__
#define __MARLIN_SPI_H__

#include <stdint.h>
#include "softspi.h"

// make sure SPCR rate is in expected bits
#if (SPR0 != 0 || SPR1 != 1)
  #error "unexpected SPCR bits"
#endif

// SPI speed is F_CPU/2^(1 + index), 0 <= index <= 6
uint8_t const SPI_FULL_SPEED = 0,         // Set SCK to max rate of F_CPU/2
              SPI_HALF_SPEED = 1,         // Set SCK rate to F_CPU/4
              SPI_QUARTER_SPEED = 2,      // Set SCK rate to F_CPU/8
              SPI_EIGHTH_SPEED = 3,       // Set SCK rate to F_CPU/16
              SPI_SIXTEENTH_SPEED = 4,    // Set SCK rate to F_CPU/32
              SPI_MSBFIRST = 1,           // Set data order to most significant bit first
              SPI_LSBFIRST = 0,           // Set data order to least significant bit first
              SPI_MODE0 = 0,              // CPOL 0, CPHA 0
              SPI_MODE1 = 1,              // CPOL 0, CPHA 1
              SPI_MODE2 = 2,              // CPOL 1, CPHA 0
              SPI_MODE3 = 3;              // CPOL 1, CPHA 1


template<uint8_t MisoPin, uint8_t MosiPin, uint8_t SckPin>
class SPI {
  static SoftSPI<MisoPin, MosiPin, SckPin> softSPI;
  public:
    FORCE_INLINE static void init() { softSPI.begin(); }
    FORCE_INLINE static void send(uint8_t data) { softSPI.send(data); }
    FORCE_INLINE static uint8_t receive() { return softSPI.receive(); }
};


// Hardware SPI
template<>
class SPI<MISO_PIN, MOSI_PIN, SCK_PIN> {
  public:
    FORCE_INLINE static void init() {
        OUT_WRITE(SCK_PIN, LOW);
        OUT_WRITE(MOSI_PIN, HIGH);
        SET_INPUT(MISO_PIN);
        WRITE(MISO_PIN, HIGH);
    }
    FORCE_INLINE static void setRate(uint8_t spiRate) {
      // See avr processor documentation
      SPCR = _BV(SPE) | _BV(MSTR) | (spiRate >> 1);
      SPSR = spiRate & 1 || spiRate == 6 ? 0 : _BV(SPI2X);
    }
    FORCE_INLINE static void setBitOrder(uint8_t bitOrder) {
      if (bitOrder == SPI_LSBFIRST) SPCR |= _BV(DORD);
      else SPCR &= ~(_BV(DORD));
    }
    FORCE_INLINE static void setDataMode(uint8_t dataMode) {
      if (dataMode == SPI_MODE0 || dataMode == SPI_MODE1) {
        // set CPOL to 0
        SPCR &= ~(_BV(CPOL));
      } else {
        // set CPOL to 1
        SPCR |= _BV(CPOL);
      }
      if (dataMode == SPI_MODE0 || dataMode == SPI_MODE2) {
        // set CPHA to 0
        SPCR &= ~(_BV(CPHA));
      } else {
        // set CPHA to 1
        SPCR |= _BV(CPHA);
      }
    }
    FORCE_INLINE static uint8_t receive() {
      SPDR = 0;
      while (!TEST(SPSR, SPIF)) { /* nada */ }
      return SPDR;
    }
    FORCE_INLINE static uint8_t transfer(uint8_t data) {
      SPDR = data;
      while (!TEST(SPSR, SPIF)) { /* nada */ }
      return SPDR;
    }
    // polls a request waiting for an expected response
    // returns true if the response arrived in time
    static bool waitResponse(uint8_t poll, uint8_t expected, unsigned long timeout) {
      uint8_t res;
      timeout += millis();
      while(timeout > millis()) {
        //SERIAL_ECHOLNPAIR("poll: ", poll);
        res = transfer(poll);
        //SERIAL_ECHOLNPAIR("res:  ", res);
        if(res==expected) {
          return true;
        } else {
          delay(200);
        }
      }
      return false;
    }

};

#endif // __MARLIN_SPI_H__
