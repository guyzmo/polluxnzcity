#ifndef __BUFFER_H__
#define __BUFFER_H__

/**
 * Pollux'NZ City Project
 * Copyright (c) 2012 CKAB, hackable:Devices
 * Copyright (c) 2012 Bernard Pratz <guyzmo{at}hackable-devices{dot}org>
 * Copyright (c) 2012 Lucas Fernandez <kasey{at}hackable-devices{dot}org>
 * 
 * Pollux'NZ City is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Pollux'NZ City is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this project. If not, see <http://www.gnu.org/licenses/>.
 */
/* thanks to Akash Nath, akashnath{at}rediffmail{dot}com for this snippet,
    taken from http://datastructures.itgo.com/staque/queues/circular        */

#ifndef BUFFER_SIZE
# define BUFFER_SIZE 32
#endif

class Buffer {
    uint8_t queue[BUFFER_SIZE];
    uint8_t front;
    uint8_t rear;

    public:
        Buffer() : front(0), rear(-1) {
            for (int i=0;i<BUFFER_SIZE;++i) queue[i] = 0;
        }

        int push(uint8_t data) {
            if(rear == BUFFER_SIZE-1)
                rear = 0;
            else
                rear = rear + 1;
            if( front == rear && queue[front] != 0)
                return -1;
            else
                queue[rear] = data;
            return rear;
        }
        int push(char* str) {
            if (strlen(str) > BUFFER_SIZE)
                return -1;
            for (uint8_t i=0;i<strlen(str);++i)
                if (size() < BUFFER_SIZE)
                    push(str[i]);
                else
                    return -1;
            return size();
        }
        uint8_t pop() {
            int popped_front = 0;
            if (front == rear+1)
                return -1;
            else
            {
                popped_front = queue[front];
                queue[front] = 0;
                if (front == BUFFER_SIZE-1)
                    front = 0;
                else
                    front = front + 1;
            }
            return popped_front;
        }
        int size() {
            if ((rear-front+1) < 0 || (rear-front+1) > BUFFER_SIZE)
                return 0;
            return rear-front+1;
        }
        void clear() {
            front = 0;
            rear = -1;
            for (int i=0;i<BUFFER_SIZE;++i) queue[i] = 0;
        }
};

#endif // __BUFFER_H__
