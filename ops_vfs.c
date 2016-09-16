/**
 *@file ops_vfs.c
 *@brief file originally from e_vfs
 *@details
 *@date Created on 2016年3月26日
 *@author Administrator
 *@par CopyRight(C)  Private
 *@par TODO
 */

#include "ops_vfs.h"
#include "string.h"


vfs_struct sys_devices[VFS_DEV_TOTAL];///<device objs representation in sys
int errno;///<state that be set after 'vfs' operation
/**
 * @brief find the device path point to
 * @param[in] path device name and path.
 * @return file description
 */
int _find_device_by_name(const char * path)
{
	int fd = -1;
	int sys_itr = -1;
	int st = -1;
	while(sys_itr < VFS_DEV_TOTAL)
	{
		if(!PTR_VALID(path))
		{
			fd = -1;
			break;
		}
		sys_itr++;
		if(!PTR_VALID(sys_devices[sys_itr].name_ptr))
			continue;

		st = strcmp(path,sys_devices[sys_itr].name_ptr);
		if(st == 0)
		{
			fd = sys_itr;
			break;
		}
	}

	return fd;
}
/**
 * @brief implementation of POSIX'OPEN'
 * @param[in] path device path and/or name
 * @param[in] flag set attributes of opened device
 * @return file description
 */
int open(const char * path,int flag)
{
	int fd = -1;
	int rtn =-1;
	if(ACCSS_MODE_VALID(flag))
	{
		fd = _find_device_by_name(path);
		rtn = sys_devices[fd].open(path,flag);
		if((fd>-1) && (rtn == 0) )
		{
			get_ref(fd);///< 后续要添加此处的判断
			sys_devices[fd].fd  = fd;
		}
		else
		{
			fd = - 1;
			//memset(&sys_devices[fd],0,sizeof(sys_devices[fd]));
			errno = -ENODEV;
		}
	}
	else
	{
		errno = -EACCES;
	}

	return fd;
}

/**
 * @brief vfs read implementation
 * @param[in] fd  get data from the device which fd represent
 * @param[out] buf the data to put in
 * @param[in] count numbers of gotten data
 * @return >-1:the number of bytes read is returned (zero indicates
 * end of file), and the file position is advanced by this number.
 * @return -1:fail,errno is set appropriately. In this case it is
 * left unspecified whether the file position (if any) changes.
 */
ssize_t read(int fd, void *buf, size_t count)
{
	int rtn = -1;
	if(FD_VALID(fd))
	{
		if(count>0)
		{
			if(PTR_VALID(buf))
			{
				if(PTR_VALID(sys_devices[fd].read))
					rtn = sys_devices[fd].read(fd,buf,count);
			}
			else
			{
				errno = -EFAULT;
			}
		}
		else
		{
			rtn = 0;
		}
	}
	else
	{
		errno = -EBADF;
	}

	return rtn;
}

/**
 * @brief vfs write implementation
 * @param[in] fd  send data to the device which fd represent
 * @param[in] buf where store data
 * @param[in] count how many data written in bytes
 * @returns >-1:the number of bytes written is returned (zero
 * indicates that nothing was written,
 * @return -1:fail, in which case errno is set to indicate the error.
 */
ssize_t write(int fd, const void *buf, size_t count)
{
	int rtn = -1;

	if(FD_VALID(fd))
	{
		if(count>0)
		{
			if(PTR_VALID(buf))
			{
				if(PTR_VALID(sys_devices[fd].write))
					rtn = sys_devices[fd].write(fd,buf,count);
				else
					errno = -EFAULT ;
			}
			else
			{
				errno = -ENOSPC ;
			}
		}
		else
		{
			rtn = 0;
		}
	}
	else
	{
		errno = -EBADF;
	}

	return rtn;
}

/**
 * @brief control the I/O device accoss the para 'request' & 'arg'
 * @param fd file description which to control
 * @param request indicate the way to control
 * @param arg	indicate the thing 'IN/OUT/VOID',and the  data corresponding data too the thing;whereas its
 *  type usually seen as 'char*'/'void*'
 * @return 0:success,
 * @return -1:fail,and errno is set appropriately.
 */
#ifdef FULL_SUPPORT
int ioctl(int fd, int request, ...)
{
		int rtn = -1;
		return rtn;
}
#else
int ioctl(int fd, int request, unsigned long arg)
{
	int rtn = -1;
	if(FD_VALID(fd))
	{
		if(PTR_VALID((void*)arg))
			rtn = sys_devices[fd].ioctl(fd,request,arg);
		else
			errno = -EINVAL;
	}
	else
	{
		errno = -EBADF;
	}
	return rtn;
}
#endif

/**
 * @brief close the device
 * @param fd the file description which to close
 * @return
 */
int close(int fd)
{
	int rtn = -1 ;
	if(FD_VALID(fd))
	{
		if(PTR_VALID(sys_devices[fd].close))
			rtn = sys_devices[fd].close(fd);
		if(rtn == 0)
		{
			put_ref(fd);
			memset((char*)&sys_devices[fd],0,sizeof(sys_devices));
		}
		else
		{
			errno = -EIO;
		}
	}
	else
	{
		errno = -EBADF;
	}

	return rtn;
}
