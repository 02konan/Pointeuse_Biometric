// 
//

#include "stdafx.h"
#include "MimeProtocolParsing.h"

char *get_boundary(char *mfd)
{
	char *p, *end;
	char *boundary = NULL;
	int boundary_size;

	if ((p = strstr(mfd, "boundary=")) == NULL) {
		return NULL;
	}

	p += 9;
	if ((end = strstr(p, "\r\n")) == NULL) {
		return NULL;
	}

	boundary_size = end - p + 1;
    if (boundary_size < 1)
    {
        return NULL;
    }

	boundary = new char[boundary_size];
    if (NULL == boundary)
    {
        return NULL;
    }

    strncpy(boundary, p, boundary_size);
    boundary[boundary_size - 1] = 0;

    return boundary;
}
/*
内存字符串比对
返回比对到的位置内存指针
*/
char* memstr(char* full_data, int full_data_len, char* substr)
{
    if (full_data == NULL || full_data_len <= 0 || substr == NULL) {
        return NULL;
    }

    if (*substr == '\0') {
        return NULL;
    }

    int sublen = strlen(substr);

    int i;
    char* cur = full_data;
    int last_possible = full_data_len - sublen + 1;
    for (i = 0; i < last_possible; i++) {
        if (*cur == *substr) {
            //assert(full_data_len - i >= sublen);  
            if (memcmp(cur, substr, sublen) == 0) {
                //found  
                return cur;
            }
        }
        cur++;
    }

    return NULL;
}

/*
表单式文本组合内容解析
*/
char* mutipart_form_data(char *mfd, char *boundary, char **type, char **content, char **file, int mfdSize, int& contentLen, int& offsetLen)
{
    int iDataSize = mfdSize; //报文长度
    char *p, *end, *filename, *realEnd = NULL;
    int size = 0;
    int boundary_size = 0;

    char szBoundary[128] = { 0 };
    char szBoundaryEnd[128] = { 0 };
    sprintf(szBoundary, "--%s", boundary);
    boundary_size = strlen(szBoundary);

    *content = *type = *file = NULL;
    contentLen = offsetLen = 0;
    char* mfdTemp = mfd;
    if ((p = memstr(mfd, iDataSize, szBoundary)) == NULL) {
        return NULL;
    }
    else if (*(p + boundary_size) == '-' && *(p + boundary_size + 1) == '-')//判断p是否为最终的结束boundary,是的话直接return，防止在strchr出现概率性崩溃
    {
        return NULL;
    }
    p += (boundary_size + 2);
    iDataSize = mfdSize - (p - mfd); //修改剩余长度
    //解析type
    if ((p = strchr(p, '/')) == NULL) {
        return NULL;
    }

    if ((end = strchr(p, '\r')) == NULL)
    {
        return NULL;
    }
    else
    {
        if (((realEnd = strchr(p, ';')) != NULL) && (realEnd - p == 4))//考虑这种情况  Content-Type: application/xml; charset="UTF-8"
        {
            end = realEnd;
        }
    }

    size = end - p;
    if (size <= 0)
    {
        return NULL;
    }

    *type = (char*)malloc(size);
    if (NULL == *type)
    {
        return NULL;
    }

    strncpy(*type, (p + 1), size);
    (*type)[size - 1] = 0;


    if ((filename = memstr(p, iDataSize, "Content-ID:")) != NULL)
    {
        if (((realEnd = strchr(filename, '\r')) != NULL))//考虑这种情况  Content-Type: application/xml; charset="UTF-8"
        {
            end = realEnd;
        }
        size = end - filename - strlen("Content-ID:") + 1;//多扩展一字节，防止内容满存
        if (size > 0)
        {
            *file = (char*)malloc(size);
            if (NULL == *file)
            {
                return NULL;
            }

            strncpy(*file, (filename + strlen("Content-ID:")), size -1);
            (*file)[size - 1] = 0;
        }
    }
    iDataSize = mfdSize - (p - mfd); //修改剩余长度
    if ((end = memstr(p, iDataSize, "\r\n\r\n")) == NULL) {
		return NULL;
	}

	p = end + 4;
    iDataSize = mfdSize - (p - mfd); //修改剩余长度
    if ((end = memstr(p, iDataSize, szBoundary)) == NULL) {
		return NULL;
	}

	mfd = end;

	while (*p == '\r' || *p == '\n') {
		p++;
	}

	while (*end == '\r' || *end == '\n') {
		end--;
	}

	if (p > end) {
		return NULL;
	}

    if (strcmp(*type, "xml") == 0 || strcmp(*type, "XML") == 0)//如果数据格式为XML，字符串末尾只需排除一个'\n'即可
    {
        size = end - p - 1;
    }
    else
    {
        size = end - p - 2;
    }
	
    if (size <= 0)
    {
        return NULL;
    }

	*content = (char*)malloc(size+1);
    if (NULL == *content)
    {
        return NULL;
    }

	memcpy(*content, p, size);
	(*content)[size] = 0;
	contentLen = size;
	offsetLen = mfd - mfdTemp;
	return mfd;
}




