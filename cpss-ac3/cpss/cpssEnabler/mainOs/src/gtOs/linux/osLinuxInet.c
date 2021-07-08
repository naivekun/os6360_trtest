/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE IS A REFERENCE CODE FOR MARVELL SWITCH PRODUCTS.  IT IS PROVIDED   *
* "AS IS" WITH NO WARRANTIES, EXPRESSED, IMPLIED OR OTHERWISE, REGARDING ITS   *
* ACCURACY, COMPLETENESS OR PERFORMANCE.                                       *
* CUSTOMERS ARE FREE TO MODIFY IT AND USE IT ONLY IN THEIR PRODUCTION          *
* SOFTWARE RELEASES WITH MARVELL SWITCH CHIPSETS.                              *
*******************************************************************************/

#include <gtOs/gtGenTypes.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>

/*******************************************************************************
* osNtohl
*
* DESCRIPTION:
*       convert long integer from network order to host order
*
* INPUTS:
*       data - long integer in network order
*
* OUTPUTS:
*       None
*
* RETURNS:
*       long integer in host order
*
* COMMENTS:
*       None
*
*******************************************************************************/
unsigned long osNtohl(unsigned long netlong)
{
    return ntohl(netlong);
}

/*******************************************************************************
* osHtonl
*
* DESCRIPTION:
*       convert long  integer from host order to network order
*
* INPUTS:
*       long integer in host order
*
* OUTPUTS:
*       None
*
* RETURNS:
*       long integer in network order
*
* COMMENTS:
*       None
*
*******************************************************************************/

unsigned long osHtonl(unsigned long hostlong)
{
    return htonl(hostlong);
}

/*******************************************************************************
* osNtohs
*
* DESCRIPTION:
*       convert short integer from network order to host order
*
* INPUTS:
*       data - short integer in network order
*
* OUTPUTS:
*       None
*
* RETURNS:
*       short integer in host order
*
* COMMENTS:
*       None
*
*******************************************************************************/
unsigned int osNtohs(unsigned int netshort)
{
    return ntohs(netshort);
}

/*******************************************************************************
* osHtons
*
* DESCRIPTION:
*       convert short integer from host order to network order
*
* INPUTS:
*       short integer in host order
*
* OUTPUTS:
*       None
*
* RETURNS:
*       short integer in network order
*
* COMMENTS:
*       None
*
*******************************************************************************/
unsigned int osHtons(unsigned int hostshort)
{
    return htons(hostshort);
}

/**
* @internal osInetNtoa function
* @endinternal
*
* @brief   convert an network address to dot notation, store it in a buffer.
*
* @param[in] ipAddr                   -  inet address
*
* @param[out] buf                      -  where to return ASCII string
*/
void    osInetNtoa
(
    IN  GT_U32      ipAddr,
    OUT GT_U8       *buf
)
{
    struct in_addr in;

    if(buf == NULL)
    {
        return;
    }

    in.s_addr = htonl(ipAddr);

    strcpy((char*)buf, inet_ntoa(in));
}



