/*
 * This file implements two functions that read XML and binary information from a buffer,
 * respectively, and return pointers to Record or NULL.
 *
 * *** YOU MUST IMPLEMENT THESE FUNCTIONS ***
 *
 * The parameters and return values of the existing functions must not be changed.
 * You can add function, definition etc. as required.
 */
#include "recordFromFormat.h"
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <stdint.h>
#include <arpa/inet.h>

Record *XMLtoRecord(char *buffer, int bufSize, int *bytesread)
{

    Record *record = newRecord();
    char *content = strtok(buffer, "<>=\"");

    while (content != NULL)
    {
        if (strcmp(content, "source") == 0)
        {
            content = strtok(NULL, "<>=\"");
            setSource(record, *content);
        }
        else if (strcmp(content, "dest") == 0)
        {
            content = strtok(NULL, "<>=\"");
            setDest(record, *content);
        }
        else if (strcmp(content, "username") == 0)
        {
            content = strtok(NULL, "<>=\"");
            setUsername(record, content);
        }
        else if (strcmp(content, "id") == 0)
        {
            content = strtok(NULL, "<>=\"");
            setId(record, atoi(content));
        }
        else if (strcmp(content, "group") == 0)
        {
            content = strtok(NULL, "<>=\"");
            setGroup(record, atoi(content));
        }
        else if (strcmp(content, "semester") == 0)
        {
            content = strtok(NULL, "<>=\"");
            setSemester(record, atoi(content));
        }
        else if (strcmp(content, "grade") == 0)
        {

            content = strtok(NULL, "<>=\"");

            if (strcmp(content, "Bachelor") == 0)
            {
                setGrade(record, Grade_Bachelor);
            }
            else if (strcmp(content, "Master") == 0)
            {
                setGrade(record, Grade_Master);
            }
            else if (strcmp(content, "PhD") == 0)
            {
                setGrade(record, Grade_PhD);
            }
            else
            {
                setGrade(record, Grade_None);
            }
        }
        else if (strcmp(content, "course") == 0)
        {
            content = strtok(NULL, "<>=\"");

            if (strcmp(content, "IN1000") == 0)
            {
                setCourse(record, Course_IN1000);
            }
            else if (strcmp(content, "IN1010") == 0)
            {
                setCourse(record, Course_IN1010);
            }
            else if (strcmp(content, "IN1020") == 0)
            {
                setCourse(record, Course_IN1020);
            }
            else if (strcmp(content, "IN1030") == 0)
            {
                setCourse(record, Course_IN1030);
            }
            else if (strcmp(content, "IN1050") == 0)
            {
                setCourse(record, Course_IN1050);
            }
            else if (strcmp(content, "IN1060") == 0)
            {
                setCourse(record, Course_IN1060);
            }
            else if (strcmp(content, "IN1080") == 0)
            {
                setCourse(record, Course_IN1080);
            }
            else if (strcmp(content, "IN1140") == 0)
            {
                setCourse(record, Course_IN1140);
            }
            else if (strcmp(content, "IN1150") == 0)
            {
                setCourse(record, Course_IN1150);
            }
            else if (strcmp(content, "IN1900") == 0)
            {
                setCourse(record, Course_IN1900);
            }
            else if (strcmp(content, "IN1910") == 0)
            {
                setCourse(record, Course_IN1910);
            }
        }
        content = strtok(NULL, "<>=\"");
    }
    *bytesread = bufSize;
    return record;
}

// hentet fra Cbra
int is_set(uint32_t flag, int pos)
{
    return flag & (1 << pos);
}

Record *BinaryToRecord(char *buffer, int bufSize, int *bytesread)
{
    // Check if buffer size is sufficient to read the flags
    if (bufSize < sizeof(uint8_t))
    {
        perror("Error: Insufficient buffer size for flags");
        return NULL;
    }

    unsigned char flags = 0;
    memcpy(&flags, &buffer[*bytesread], sizeof(char));
    Record *record = newRecord();

    *bytesread += sizeof(char);

    if (flags & FLAG_SRC)
    {
        // Set the source field
        setSource(record, buffer[*bytesread]);
        *bytesread += sizeof(char);
    }

    if (flags & FLAG_DST)
    {
        // Set the destination field
        setDest(record, buffer[*bytesread]);
        *bytesread += sizeof(char);
    }

    if (flags & FLAG_USERNAME)
    {
        // Get the username length from the buffer
        int usernameLength = 0;

        memcpy(&usernameLength, &buffer[*bytesread], sizeof(int));
        usernameLength = ntohl(usernameLength);

        *bytesread += sizeof(int);
        char recordname[usernameLength + 1];

        memcpy(&recordname, &buffer[*bytesread], usernameLength);

        *bytesread += usernameLength;
        recordname[usernameLength] = '\0';
        // Set the username 
        setUsername(record, recordname);
    }

    if (flags & FLAG_ID)
    {
        // Get the ID from the buffer
        uint32_t id;
        memcpy(&id, &buffer[*bytesread], sizeof(int));

        id = ntohl(id);
        *bytesread += sizeof(uint32_t);

        // Set the ID field
        setId(record, id);
    }

    if (flags & FLAG_GROUP)
    {
        // Get the group from the buffer
        uint32_t group;
        memcpy(&group, &buffer[*bytesread], sizeof(int));
        group = ntohl(group);
        *bytesread += sizeof(uint32_t);

        setGroup(record, group);
    }

    if (flags & FLAG_SEMESTER)
    {
        setSemester(record, buffer[*bytesread]);
        *bytesread += sizeof(char);
    }

    if (flags & FLAG_GRADE)
    {
        // set the group field
        setGrade(record, (Grade)buffer[*bytesread]);
        *bytesread += sizeof(char);
    }

    if (flags & FLAG_COURSES)
    {
        int record_courses;
        memcpy(&record_courses, &buffer[*bytesread], sizeof(short));
        record_courses = ntohs(record_courses);
        if (is_set(record_courses, 0))
            setCourse(record, Course_IN1000);
        if (is_set(record_courses, 1))
            setCourse(record, Course_IN1010);
        if (is_set(record_courses, 2))
            setCourse(record, Course_IN1020);
        if (is_set(record_courses, 3))
            setCourse(record, Course_IN1030);
        if (is_set(record_courses, 4))
            setCourse(record, Course_IN1050);
        if (is_set(record_courses, 5))
            setCourse(record, Course_IN1060);
        if (is_set(record_courses, 6))
            setCourse(record, Course_IN1080);
        if (is_set(record_courses, 7))
            setCourse(record, Course_IN1140);
        if (is_set(record_courses, 8))
            setCourse(record, Course_IN1150);
        if (is_set(record_courses, 9))
            setCourse(record, Course_IN1900);
        if (is_set(record_courses, 10))
            setCourse(record, Course_IN1910);

        *bytesread += sizeof(short);
        setCourse(record, record_courses);
    }

    // Calculate the number of bytes read from the buffer
    return record;
}
