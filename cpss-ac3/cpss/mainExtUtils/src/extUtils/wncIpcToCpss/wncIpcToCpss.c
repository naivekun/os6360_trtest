#include <extUtils/wncIpcToCpss/wncIpcToCpss.h>

/* The task Tid */
CPSS_TASK wncIpcToCpssTaskId;

/* Tasks semaphores */
CPSS_OS_SIG_SEM wncIpcToCpssSingSem = 0;

static char *c_func = NULL;
static uint64_t *params = NULL, *values = NULL, *structs = NULL;

static int parseParamVariable(cJSON *j_var, uint64_t *param, uint64_t *val)
{
	int in_out, oob;
	cJSON *j_dir, *j_type, *j_value;
	char *c_dir, *c_type, *c_value;

	if (((j_dir = cJSON_GetObjectItem(j_var, "dir")) == NULL) ||
	    !cJSON_IsString(j_dir)) {
		dbg_err("oops! variable attribute \"dir\" is missing\n");
		return -1;
	}

	c_dir = cJSON_GetStringValue(j_dir);
	dbg_info("variable attribute \"dir\"=%s\n", c_dir);

	/* do not support 'out' */
	in_out = 1;
	if (!strcmp("in", c_dir)) {
		in_out = 0;
	}
	else if (strcmp("inout", c_dir)) {
		dbg_err("oops! unknown variable attribute \"dir\" %s\n", c_dir);
		return -1;
	}

	if (((j_type = cJSON_GetObjectItem(j_var, "type")) == NULL) ||
	    !cJSON_IsString(j_type)) {
		dbg_err("oops! variable attribute \"type\" is missing\n");
		return -1;
	}
	else if (((j_value = cJSON_GetObjectItem(j_var, "value")) == NULL) ||
	    !cJSON_IsString(j_value)) {
		dbg_err("oops! variable attribute \"value\" is missing\n");
		return -1;
	}

	c_type = cJSON_GetStringValue(j_type);
	c_value = cJSON_GetStringValue(j_value);
	dbg_info("variable attribute \"type\"=%s\n", c_type);
	dbg_info("variable attribute \"value\"=%s\n", c_value);

	/* parse variable */
	if (strcmp("string", c_type) == 0) {
        *param = *val = (uint64_t)strtoull(c_value, NULL, 10);
		return 0;
	}

	oob = 0; /* oob: out of boundary */
	switch (c_type[0]) {
	case 'b':
		if (strcmp("bool", c_type)) goto fail;
		*val = (uint64_t)strtoull(c_value, NULL, 10);
		oob = ((int64_t)*val < 0) || (*val > 1);
		break;
	case 'i':
		switch (c_type[1]) {
		case '8':
			if (strcmp("i8", c_type)) goto fail;
			*val = (uint64_t)strtoll(c_value, NULL, 10);
			oob = ((int64_t)*val < INT8_MIN) || (*val > INT8_MAX);
			break;
		case '1':
			if (strcmp("i16", c_type)) goto fail;
			*val = (uint64_t)strtoll(c_value, NULL, 10);
			oob = ((int64_t)*val < INT16_MIN) || (*val > INT16_MAX);
			break;
		case '3':
			if (strcmp("i32", c_type)) goto fail;
			*val = (uint64_t)strtoll(c_value, NULL, 10);
			oob = ((int64_t)*val < INT32_MIN) || (*val > INT32_MAX);
			break;
		case '6':
			if (strcmp("i64", c_type)) goto fail;
			*val = (uint64_t)strtoll(c_value, NULL, 10);
			oob = ((int64_t)*val < INT64_MIN) || (*val > INT64_MAX);
			break;
		default:
			goto fail;
		}
		break;
	case 'u':
		switch (c_type[1]) {
		case '8':
			if (strcmp("u8", c_type)) goto fail;
			*val = (uint64_t)strtoull(c_value, NULL,
				strncasecmp("0x", c_value, 2) ? 10 : 16);
			oob = (*val > UINT8_MAX);
			break;
		case '1':
			if (strcmp("u16", c_type)) goto fail;
			*val = (uint64_t)strtoull(c_value, NULL,
				strncasecmp("0x", c_value, 2) ? 10 : 16);
			oob = (*val > UINT16_MAX);
			break;
		case '3':
			if (strcmp("u32", c_type)) goto fail;
			*val = (uint64_t)strtoull(c_value, NULL,
				strncasecmp("0x", c_value, 2) ? 10 : 16);
			oob = (*val > UINT32_MAX);
			break;
		case '6':
			if (strcmp("u64", c_type)) goto fail;
			*val = (uint64_t)strtoull(c_value, NULL,
				strncasecmp("0x", c_value, 2) ? 10 : 16);
			oob = (*val > UINT64_MAX);
			break;
		default:
			goto fail;
		}
		break;
	default:
fail:
		dbg_err("oops! unknown variable attribute\n");
		return -1;
	}

	if (oob) {
		dbg_err("oops! out of boundary\n");
		return -1;
	}
	*param = in_out ? (uint64_t)val : *val;

	return 0;
}

static uint8_t* parseParamStructure(cJSON *j_struct, uint8_t *buf, uint8_t *buf_end)
{
	int count, st_sz, oob;
	cJSON *j_struct_child, *j_param, *j_type, *j_value, *j_count;
	char *c_type, *c_value;
	uint64_t val;
	uint8_t *st, *st_end;

	if (j_struct == NULL) return NULL;

	dbg_info("struct=%s\n", cJSON_PrintUnformatted(j_struct));

	cJSON_ArrayForEach(j_param, j_struct) {
		switch (cJSON_GetArraySize(j_param)) {
		case 3: /* variable */
			if (((j_type = cJSON_GetObjectItem(j_param, "type")) == NULL) ||
			    !cJSON_IsString(j_type)) {
				dbg_err("oops! struct attribute \"type\" is missing\n");
				return NULL;
			}
			else if (((j_value = cJSON_GetObjectItem(j_param, "value")) == NULL) ||
			    !cJSON_IsString(j_value)) {
				dbg_err("oops! struct attribute \"value\" is missing\n");
				return NULL;
			}
			else if (((j_count = cJSON_GetObjectItem(j_param, "count")) == NULL) ||
			    !cJSON_IsNumber(j_count)) {
				dbg_err("oops! struct attribute \"count\" is missing\n");
				return NULL;
			}
			else if ((count = (int)cJSON_GetNumberValue(j_count)) <= 0) {
				dbg_err("oops! struct attribute \"count\" must be greater than or equal to 1\n");
				return NULL;
			}

			c_type = cJSON_GetStringValue(j_type);
			c_value = cJSON_GetStringValue(j_value);
			dbg_info("struct attribute \"type\"=%s\n", c_type);
			dbg_info("struct attribute \"value\"=%s\n", c_value);
			dbg_info("struct attribute \"count\"=%d\n", count);

#define _FILL_BUF(buf, end, type, val, cnt) do { \
	int _sz = sizeof(type), _i; \
	if ((uint64_t)(buf) % _sz) { \
		dbg_err("oops! miss padding bytes sz=%d, 0x%p\n", _sz, (buf)); \
		return NULL; \
	} \
	if (((buf) + _sz * (cnt)) > (end)) { \
		dbg_err("oops! buffer is undersized\n"); \
		return NULL; \
	} \
	for (_i = 0; _i < (cnt); _i++, (buf) += _sz) \
		*(type*)(buf) = (type)(val); \
	_DEBUG_BUF(buf, type, cnt); \
} while (0)


#define _DEBUG_BUF(buf, type, cnt) do { \
	int _sz = sizeof(type), _i; \
	uint8_t *_ptr = (buf) - _sz * (cnt); \
	dbg_info("buf addr=0x%p,element=[", _ptr); \
	if (_sz > (int)sizeof(int64_t)) { \
		for (_i = 0; _i < ((cnt) - 1); _i++, _ptr += _sz) \
			dbg_info("%ld,", (uint64_t)(*(type*)_ptr)); \
		dbg_info("%ld],count=%d\n", (uint64_t)(*(type*)_ptr), (cnt)); \
	} \
	else { \
		for (_i = 0; _i < ((cnt) - 1); _i++, _ptr += _sz) \
			dbg_info("%ld,", (int64_t)(*(type*)_ptr)); \
		dbg_info("%ld],count=%d\n", (int64_t)(*(type*)_ptr), (cnt)); \
	} \
} while (0)

			/* parse variable */
			oob = 0; /* oob: out of boundary */
			switch (c_type[0]) {
			case 'b':
				if (strcmp("bool", c_type)) goto fail;
				val = (uint64_t)strtoull(c_value, NULL, 10);
				oob = ((int64_t)val < 0) || (val > 1);
				_FILL_BUF(buf, buf_end, GT_BOOL, val, count);
				break;
			case 'i':
				switch (c_type[1]) {
				case '8':
					if (strcmp("i8", c_type)) goto fail;
					val = (uint64_t)strtoll(c_value, NULL, 10);
					oob = ((int64_t)val < INT8_MIN) || (val > INT8_MAX);
					_FILL_BUF(buf, buf_end, int8_t, val, count);
					break;
				case '1':
					if (strcmp("i16", c_type)) goto fail;
					val = (uint64_t)strtoll(c_value, NULL, 10);
					oob = ((int64_t)val < INT16_MIN) || (val > INT16_MAX);
					_FILL_BUF(buf, buf_end, int16_t, val, count);
					break;
				case '3':
					if (strcmp("i32", c_type)) goto fail;
					val = (uint64_t)strtoll(c_value, NULL, 10);
					oob = ((int64_t)val < INT32_MIN) || (val > INT32_MAX);
					_FILL_BUF(buf, buf_end, int32_t, val, count);
					break;
				case '6':
					if (strcmp("i64", c_type)) goto fail;
					val = (uint64_t)strtoll(c_value, NULL, 10);
					oob = ((int64_t)val < INT64_MIN) || (val > INT64_MAX);
					_FILL_BUF(buf, buf_end, int64_t, val, count);
					break;
				default:
					goto fail;
				}
				break;
			case 'u':
				switch (c_type[1]) {
				case '8':
					if (strcmp("u8", c_type)) goto fail;
					val = (uint64_t)strtoull(c_value, NULL,
						strncasecmp("0x", c_value, 2) ? 10 : 16);
					oob = (val > UINT8_MAX);
					_FILL_BUF(buf, buf_end, uint8_t, val, count);
					break;
				case '1':
					if (strcmp("u16", c_type)) goto fail;
					val = (uint64_t)strtoull(c_value, NULL,
						strncasecmp("0x", c_value, 2) ? 10 : 16);
					oob = (val > UINT16_MAX);
					_FILL_BUF(buf, buf_end, uint16_t, val, count);
					break;
				case '3':
					if (strcmp("u32", c_type)) goto fail;
					val = (uint64_t)strtoull(c_value, NULL,
						strncasecmp("0x", c_value, 2) ? 10 : 16);
					oob = (val > UINT32_MAX);
					_FILL_BUF(buf, buf_end, uint32_t, val, count);
					break;
				case '6':
					if (strcmp("u64", c_type)) goto fail;
					val = (uint64_t)strtoull(c_value, NULL,
						strncasecmp("0x", c_value, 2) ? 10 : 16);
					oob = (val > UINT64_MAX);
					_FILL_BUF(buf, buf_end, uint64_t, val, count);
					break;
				default:
					goto fail;
				}
				break;
			default:
fail:
				dbg_err("oops! unknown struct attribute\n");
				return NULL;
			}

			if (oob) {
				dbg_err("oops! out of boundary\n");
				return NULL;
			}
			break;

		case 2: /* nested structure */
			if ((j_count = cJSON_GetObjectItem(j_param, "count")) == NULL) {
				dbg_err("oops! struct attribute \"count\" is missing\n");
				return NULL;
			}
			else if ((count = (int)cJSON_GetNumberValue(j_count)) <= 0) {
				dbg_err("oops! struct attribute \"count\" must be greater than or equal to 1\n");
				return NULL;
			}
			else if ((j_struct_child = cJSON_GetObjectItem(j_param, "struct")) == NULL) {
				dbg_err("oops! struct attribute \"struct\" is missing\n");
				return NULL;
			}
			/* parse nested structure */
			else if ((st = parseParamStructure(j_struct_child, buf, buf_end)) == NULL) {
				dbg_err("oops! unexpected struct attribute\n");
				return NULL;
			}

			/* replicate structure (st_cnt -1) times */
			if (count == 1) {buf = st; break;}
			st_sz = (int)(st - buf);
			if ((st_end = buf + st_sz * count) > buf_end) {
				dbg_err("oops! buffer is undersized\n");
				return NULL;
			}
			for (; st < st_end; st += st_sz)
				memcpy(st, buf, st_sz);
			buf = st;
			break;

		default:
			dbg_err("oops! unexpected struct attribute\n");
			return NULL;
		}
	}

	return buf;
}

static int funcExecByJsonArray(char *json) {
	int ret = 0, i = 0, ar_sz, st_sz, st_cnt, max_struct_cnt;
	cJSON *array = NULL, *j_func, *j_params, *j_param, *j_struct, *j_count;
	uint8_t *st, *st_end, *buf, *buf_end;

    CPSS_API_FUNC_PTR funcPtr;
    GT_STATUS retval;

	if ((array = cJSON_Parse(json)) == NULL) {
		dbg_err("oops! invalid JSON format (%s)\n", json);
		ret = -1; goto exit;
	}

	dbg_info("JSON=%s\n", cJSON_Print(array));

	if ((cJSON_GetArraySize(array) != 2) ||
	    ((j_func = cJSON_GetObjectItem(array, "func")) == NULL) ||
	    ((j_params = cJSON_GetObjectItem(array, "params")) == NULL) ||
	    !cJSON_IsString(j_func) || !cJSON_IsArray(j_params)) {
		dbg_err("oops! unexpected JSON content\n");
		ret = -1; goto exit;
	}

	ar_sz = cJSON_GetArraySize(j_params);
	if ((ar_sz == 0) || (ar_sz > N_MAX_PARAM)) {
		dbg_err("oops! param number exceeds %d\n", N_MAX_PARAM);
		ret = -1; goto exit;
	}

	params = calloc(N_MAX_PARAM, sizeof(uint64_t*));
	values = calloc(N_MAX_PARAM, sizeof(uint64_t*));
	structs = calloc(N_MAX_PARAM, sizeof(uint64_t*));
    if ((params == NULL) || (values == NULL) || (structs == NULL)) {
		dbg_err("oops! out of memory\n");
		ret = -1; goto exit;
	}

	c_func = cJSON_GetStringValue(j_func);
	dbg_info("function=%s\n", c_func);

	cJSON_ArrayForEach(j_param, j_params) {
		ar_sz = cJSON_GetArraySize(j_param);
		dbg_info("param[%d]=%s, object number=%d\n", i + 1, cJSON_PrintUnformatted(j_param), ar_sz);

		switch (ar_sz) {
		case 3: /* variable */
			/* parse variable */
			if ((ret = parseParamVariable(j_param, &params[i], &values[i])) == -1)
				goto exit;
			break;

		case 2: /* structure */
			if ((j_count = cJSON_GetObjectItem(j_param, "count")) == NULL) {
				dbg_err("oops! param attribute \"count\" is missing\n");
				ret = -1; goto exit;
			}
			else if ((st_cnt = (int)cJSON_GetNumberValue(j_count)) <= 0) {
				dbg_err("oops! param attribute \"count\" must be greater than or equal to 1\n");
				ret = -1; goto exit;
			}
			else if ((j_struct = cJSON_GetObjectItem(j_param, "struct")) == NULL) {
				dbg_err("oops! param attribute \"struct\" is missing\n");
				ret = -1; goto exit;
			}
			else if ((buf = (uint8_t*)calloc(MAX_STRUCT_BUFSZ, 1)) == NULL) {
				dbg_err("oops! out of memory\n");
				ret = -1; goto exit;
			}

			/* parse structure */
            structs[i] = params[i] = (uint64_t)buf;
			buf_end = buf + MAX_STRUCT_BUFSZ - 1;
			if ((st = parseParamStructure(j_struct, buf, buf_end)) == NULL) {
				ret = -1; goto exit;
			}

			/* replicate structure (st_cnt -1) times */
			if (st_cnt == 1) break;
			st_sz = (int)(st - buf);
			if ((st_end = buf + st_sz * st_cnt) > buf_end) {
				dbg_err("oops! buffer is undersized\n");
				ret = -1; goto exit;
			}
			for (; st < st_end; st += st_sz)
				memcpy(st, buf, st_sz);
			break;

		default:
			dbg_err("oops! unexpected param\n");
			ret = -1; goto exit;
		}

		i++;
	}

	/* call one of test functions */
	do {
        funcPtr = (CPSS_API_FUNC_PTR)osShellGetFunctionByName((const char*)c_func);
        if (!funcPtr) {
            dbg_err("oops! unknown func %s\n", c_func);
            goto exit;
        }

        retval = funcPtr(
                    (GT_UINTPTR)params[0],
                    (GT_UINTPTR)params[1],
                    (GT_UINTPTR)params[2],
                    (GT_UINTPTR)params[3],
                    (GT_UINTPTR)params[4],
                    (GT_UINTPTR)params[5],
                    (GT_UINTPTR)params[6],
                    (GT_UINTPTR)params[7],
                    (GT_UINTPTR)params[8],
                    (GT_UINTPTR)params[9]);

        if (retval) {
            dbg_warn("retval of %s() is [%d]\n", c_func, retval);
            ret = (int)retval;
            /* FIX: to handle ret value */
        }


    } while (0);

exit:
	if (array) cJSON_Delete(array);
	if (params) free(params);
	if (values) free(values);
	if (structs) {
        max_struct_cnt = i;
		/*for (i = 0; i < N_MAX_PARAM; i++) {*/
        for (i = 0; i < max_struct_cnt; i++) {
			if (structs[i])
				free((void*)structs[i]);
		}
		free(structs);
	}
	return ret;
}

GT_STATUS OpenCpssLocalSocket(void)
{
    int socket_fd;
    struct sockaddr_un name;

    socket_fd = socket(PF_UNIX, SOCK_STREAM, 0);
    if(socket_fd < 0)
    {
        handle_error("socket");
    }

    memset(&name, 0, sizeof(name));
    name.sun_family = AF_UNIX;
    strncpy(name.sun_path, LSOCKET_TO_CPSS, sizeof(name.sun_path));
    name.sun_path[strlen(name.sun_path)] = '\0';

    if(bind(socket_fd, (struct sockaddr *)&name, sizeof(struct sockaddr_un)) == -1)
    {
        close(socket_fd);
        handle_error("bind");
    }

    return socket_fd;
}

static unsigned __TASKCONV taskIpcToCpssStart
(
    GT_VOID *param
)
{
    int socket_fd, cli_socket;
    int ret;
    char recv_buf[STR_BUF];
    char ret_mesg[32];

    socket_fd = OpenCpssLocalSocket();
    if(GT_ERROR == socket_fd)
    {
        return GT_ERROR;
    }

    if(listen(socket_fd, LISTEN_BACKLOG) == -1)
    {
        close(socket_fd);
        handle_error("listen");
    }

    while (1) 
    {
        cli_socket = accept(socket_fd, NULL, NULL);
        if(-1 == cli_socket)
        {
            cpssOsPrintf("accept() fail!\n");
            break;
        }

        memset(recv_buf, 0, sizeof(recv_buf));
        memset(ret_mesg, 0, sizeof(ret_mesg));
        ret = read(cli_socket, recv_buf, sizeof(recv_buf));
        if(ret > 0)
        {
            /* got a message */
            if(ret >= sizeof(recv_buf))
            {
                recv_buf[sizeof(recv_buf) - 1] = '\0';
            }

            if(!strcmp("stopIpcToCpss", (char *)recv_buf))
            {
                cpssOsPrintf("close local socket to CPSS!\n");
                cpssOsSnprintf(ret_mesg, sizeof(ret_mesg), "%d", GT_OK);
                ret = write(cli_socket, ret_mesg, strlen(ret_mesg));
                if(ret < 0)
                {
                    handle_error("write to cli_socket");
                }
                close(cli_socket);
                break;
            }
            else
            {
                ret = funcExecByJsonArray((char *)&recv_buf);
                cpssOsSnprintf(ret_mesg, sizeof(ret_mesg), "%d", ret);
                ret = write(cli_socket, ret_mesg, strlen(ret_mesg));
                if(ret < 0)
                {
                    handle_error("write to cli_socket");
                }
            }
        }
        close(cli_socket);
    }
    close(socket_fd);
    unlink(LSOCKET_TO_CPSS);

    cpssOsSigSemDelete(wncIpcToCpssSingSem);
    wncIpcToCpssSingSem = 0;

    return GT_OK;
}

GT_STATUS wncIpcToCpssStart(void)
{
    GT_STATUS   rc = GT_OK;	

    if(0 == wncIpcToCpssSingSem)
    {
        if(cpssOsSigSemBinCreate("wncIpcToCpssSingSem", CPSS_OS_SEMB_EMPTY_E, &wncIpcToCpssSingSem) != GT_OK)
            return GT_NO_RESOURCE;

        rc = cpssOsTaskCreate("wncIpcToCpss",           /* Task Name      */
                               10,                      /* Task Priority  */
                               0x20000,                 /* Stack Size     */
                               taskIpcToCpssStart,      /* Starting Point */
                               NULL,                    /* Arguments list */
                               &wncIpcToCpssTaskId);    /* Task ID        */
        if(rc != GT_OK)
        {
            cpssOsPrintf("Task create fail!\n");
        }
    }
    else
    {
        cpssOsPrintf("Task IPC to CPSS has been created!\n");
    }

    return rc;
}
