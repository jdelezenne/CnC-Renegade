/*
**	Command & Conquer Renegade(tm)
**	Copyright 2025 Electronic Arts Inc.
**
**	This program is free software: you can redistribute it and/or modify
**	it under the terms of the GNU General Public License as published by
**	the Free Software Foundation, either version 3 of the License, or
**	(at your option) any later version.
**
**	This program is distributed in the hope that it will be useful,
**	but WITHOUT ANY WARRANTY; without even the implied warranty of
**	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**	GNU General Public License for more details.
**
**	You should have received a copy of the GNU General Public License
**	along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

/*********************************************************************************************** 
 ***              C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S               *** 
 *********************************************************************************************** 
 *                                                                                             * 
 *                 Project Name : Command & Conquer                                            * 
 *                                                                                             * 
 *                     $Archive:: /G/wwlib/lcw.cpp                                            $* 
 *                                                                                             * 
 *                      $Author:: Neal_k                                                      $*
 *                                                                                             * 
 *                     $Modtime:: 10/04/99 10:25a                                             $*
 *                                                                                             * 
 *                    $Revision:: 4                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------* 
 * Functions:                                                                                  * 
 *   LCW_Comp -- Performes LCW compression on a block of data.                                 * 
 *   LCW_Uncomp -- Decompress an LCW encoded data block.                                       *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include	"always.h"
#include	"lcw.h"
#include <algorithm>
#include <cstring>
#include <unordered_map>
#include <vector>

/***************************************************************************
 * LCW_Uncomp -- Decompress an LCW encoded data block.                     *
 *                                                                         *
 * Uncompress data to the following codes in the format b = byte, w = word *
 * n = byte code pulled from compressed data.                              *
 *                                                                         *
 *   Command code, n        |Description                                   *
 * ------------------------------------------------------------------------*
 * n=0xxxyyyy,yyyyyyyy      |short copy back y bytes and run x+3 from dest *
 * n=10xxxxxx,n1,n2,...,nx+1|med length copy the next x+1 bytes from source*
 * n=11xxxxxx,w1            |med copy from dest x+3 bytes from offset w1   *
 * n=11111111,w1,w2         |long copy from dest w1 bytes from offset w2   *
 * n=11111110,w1,b1         |long run of byte b1 for w1 bytes              *
 * n=10000000               |end of data reached                           *
 *                                                                         *
 *                                                                         *
 * INPUT:                                                                  *
 *      void * source ptr                                                  *
 *      void * destination ptr                                             *
 *      unsigned long length of uncompressed data                          *
 *                                                                         *
 *                                                                         *
 * OUTPUT:                                                                 *
 *     unsigned long # of destination bytes written                        *
 *                                                                         *
 * WARNINGS:                                                               *
 *     3rd argument is dummy. It exists to provide cross-platform          *
 *      compatibility. Note therefore that this implementation does not    *
 *      check for corrupt source data by testing the uncompressed length.  *
 *                                                                         *
 * HISTORY:                                                                *
 *    03/20/1995 IML : Created.                                            *
 *=========================================================================*/
int LCW_Uncomp(void const * source, void * dest, unsigned long )
{
	unsigned char * source_ptr, * dest_ptr, * copy_ptr;
	unsigned char op_code, data;
	unsigned count;
	unsigned * word_dest_ptr;
	unsigned word_data;

	/* Copy the source and destination ptrs. */
	source_ptr = (unsigned char*) source;
	dest_ptr   = (unsigned char*) dest;

	for (;;) {

		/* Read in the operation code. */
		op_code = *source_ptr++;

		if (!(op_code & 0x80)) {

			/* Do a short copy from destination. */
			count = (op_code >> 4) + 3;
			copy_ptr = dest_ptr - ((unsigned) *source_ptr++ + (((unsigned) op_code & 0x0f) << 8));

			while (count--) *dest_ptr++ = *copy_ptr++;

		} else {

			if (!(op_code & 0x40)) {

				if (op_code == 0x80) {

					/* Return # of destination bytes written. */
					return ((unsigned long) (dest_ptr - (unsigned char*) dest));

				} else {

					/* Do a medium copy from source. */
					count = op_code & 0x3f;

					while (count--) *dest_ptr++ = *source_ptr++;
				}

			} else {

				if (op_code == 0xfe) {

					/* Do a long run. */
					count = *source_ptr + ((unsigned) *(source_ptr + 1) << 8);
					data = *(source_ptr + 2);
					source_ptr += 3;
					std::memset(dest_ptr, data, count);
					dest_ptr += count;

				} else {

					if (op_code == 0xff) {

						/* Do a long copy from destination. */
						count = *source_ptr + ((unsigned) *(source_ptr + 1) << 8);
						copy_ptr = (unsigned char*) dest + *(source_ptr + 2) + ((unsigned) *(source_ptr + 3) << 8);
						source_ptr += 4;

						while (count--) *dest_ptr++ = *copy_ptr++;

					} else {

						/* Do a medium copy from destination. */
						count = (op_code & 0x3f) + 3;
						copy_ptr = (unsigned char*) dest + *source_ptr + ((unsigned) *(source_ptr + 1) << 8);
						source_ptr += 2;

						while (count--) *dest_ptr++ = *copy_ptr++;
					}
				}
			}
		}
	}
}


#if defined(_MSC_VER)


/*********************************************************************************************** 
 * LCW_Comp -- Performes LCW compression on a block of data.                                   * 
 *                                                                                             * 
 *    This routine will compress a block of data using the LCW compression method. LCW has     * 
 *    the primary characteristic of very fast uncompression at the expense of very slow        * 
 *    compression times.                                                                       * 
 *                                                                                             * 
 * INPUT:   source   -- Pointer to the source data to compress.                                * 
 *                                                                                             * 
 *          dest     -- Pointer to the destination location to store the compressed data       * 
 *                      to.                                                                    * 
 *                                                                                             * 
 *          datasize -- The size (in bytes) of the source data to compress.                    * 
 *                                                                                             * 
 * OUTPUT:  Returns with the number of bytes of output data stored into the destination        * 
 *          buffer.                                                                            * 
 *                                                                                             * 
 * WARNINGS:   Be sure that the destination buffer is big enough. The maximum size required    * 
 *             for the destination buffer is (datasize + datasize/128).                        * 
 *                                                                                             * 
 * HISTORY:                                                                                    * 
 *   05/20/1997 JLB : Created.                                                                 * 
 *=============================================================================================*/
/*ARGSUSED*/
int LCW_Comp(void const * source, void * dest, int datasize)
{
    if (datasize <= 0) { *static_cast<unsigned char*>(dest) = 0x80; return 1; }
    const auto* input = static_cast<const unsigned char*>(source);
    auto* output = static_cast<unsigned char*>(dest);
    auto* begin = output;
    std::unordered_map<unsigned, int> last;
    std::vector<int> previous(datasize, -1);
    auto key = [&](int i) { return input[i] | (unsigned(input[i+1])<<8) | (unsigned(input[i+2])<<16); };
    auto remember = [&](int first, int end) {
        for (int i = first; i < end && i+2 < datasize; ++i) {
            unsigned value = key(i);
            auto found = last.find(value);
            previous[i] = found == last.end() ? -1 : found->second;
            last[value] = i;
        }
    };
    auto word = [&](unsigned value) { *output++ = static_cast<unsigned char>(value); *output++ = static_cast<unsigned char>(value>>8); };
    int literal = 0, position = 0;
    auto flush = [&] {
        while (literal < position) {
            int count = std::min(63, position-literal);
            *output++ = static_cast<unsigned char>(0x80|count);
            std::memcpy(output, input+literal, count); output += count; literal += count;
        }
    };
    while (position < datasize) {
        int run = 1;
        while (run < 65535 && position+run < datasize && input[position+run] == input[position]) ++run;
        if (run >= 65) {
            flush(); *output++ = 0xfe; word(run); *output++ = input[position];
            remember(position, position+run); position += run; literal = position; continue;
        }
        int length = 0, match = 0;
        if (position+2 < datasize) {
            auto found = last.find(key(position));
            if (found != last.end()) for (int candidate = found->second; candidate >= 0; candidate = previous[candidate]) {
                if (candidate > 65535) continue;
                int count = 3;
                while (count < 65535 && position+count < datasize && input[candidate+count] == input[position+count]) ++count;
                if (count > length) { length = count; match = candidate; }
                if (position+count == datasize) break;
            }
        }
        unsigned offset = position-match;
        if (length >= 4 || (length == 3 && offset < 4096)) {
            flush();
            if (length <= 10 && offset < 4096) { *output++ = static_cast<unsigned char>(((length-3)<<4)|(offset>>8)); *output++ = static_cast<unsigned char>(offset); }
            else if (length <= 64) { *output++ = static_cast<unsigned char>(0xc0|(length-3)); word(match); }
            else { *output++ = 0xff; word(length); word(match); }
            remember(position, position+length); position += length; literal = position;
        } else { remember(position, position+1); ++position; }
    }
    flush(); *output++ = 0x80;
    return static_cast<int>(output-begin);
}
#endif


