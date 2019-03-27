/*-
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright (c) 2007-2009 Google Inc. and Amit Singh
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 * * Redistributions of source code must retain the above copyright
 *   notice, this list of conditions and the following disclaimer.
 * * Redistributions in binary form must reproduce the above
 *   copyright notice, this list of conditions and the following disclaimer
 *   in the documentation and/or other materials provided with the
 *   distribution.
 * * Neither the name of Google Inc. nor the names of its
 *   contributors may be used to endorse or promote products derived from
 *   this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * Copyright (C) 2005 Csaba Henk.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include <sys/param.h>
#include <sys/module.h>
#include <sys/systm.h>
#include <sys/errno.h>
#include <sys/kernel.h>
#include <sys/conf.h>
#include <sys/uio.h>
#include <sys/malloc.h>
#include <sys/queue.h>
#include <sys/lock.h>
#include <sys/mutex.h>
#include <sys/sdt.h>
#include <sys/sx.h>
#include <sys/proc.h>
#include <sys/mount.h>
#include <sys/vnode.h>
#include <sys/namei.h>
#include <sys/stat.h>
#include <sys/unistd.h>
#include <sys/filedesc.h>
#include <sys/file.h>
#include <sys/fcntl.h>
#include <sys/dirent.h>
#include <sys/bio.h>
#include <sys/buf.h>
#include <sys/sysctl.h>
#include <sys/priv.h>

#include "fuse.h"
#include "fuse_file.h"
#include "fuse_internal.h"
#include "fuse_ipc.h"
#include "fuse_node.h"
#include "fuse_file.h"

SDT_PROVIDER_DECLARE(fuse);
/* 
 * Fuse trace probe:
 * arg0: verbosity.  Higher numbers give more verbose messages
 * arg1: Textual message
 */
SDT_PROBE_DEFINE2(fuse, , internal, trace, "int", "char*");

#ifdef ZERO_PAD_INCOMPLETE_BUFS
static int isbzero(void *buf, size_t len);

#endif

/* access */

int
fuse_internal_access(struct vnode *vp,
    mode_t mode,
    struct fuse_access_param *facp,
    struct thread *td,
    struct ucred *cred)
{
	int err = 0;
	uint32_t mask = 0;
	int dataflags;
	int vtype;
	struct mount *mp;
	struct fuse_dispatcher fdi;
	struct fuse_access_in *fai;
	struct fuse_data *data;

	/* NOT YET DONE */
	/*
	 * If this vnop gives you trouble, just return 0 here for a lazy
	 * kludge.
	 */
	/* return 0;*/

	mp = vnode_mount(vp);
	vtype = vnode_vtype(vp);

	data = fuse_get_mpdata(mp);
	dataflags = data->dataflags;

	if ((mode & VWRITE) && vfs_isrdonly(mp)) {
		return EACCES;
	}
	/* Unless explicitly permitted, deny everyone except the fs owner. */
	    if (vnode_isvroot(vp) && !(facp->facc_flags & FACCESS_NOCHECKSPY)) {
		if (!(dataflags & FSESS_DAEMON_CAN_SPY)) {
			int denied = fuse_match_cred(data->daemoncred,
			    cred);

			if (denied) {
				return EPERM;
			}
		}
		facp->facc_flags |= FACCESS_NOCHECKSPY;
	}
	if (!(facp->facc_flags & FACCESS_DO_ACCESS)) {
		return 0;
	}
	if (((vtype == VREG) && (mode & VEXEC))) {
#ifdef NEED_MOUNT_ARGUMENT_FOR_THIS
		/* Let	 the kernel handle this through open / close heuristics.*/
		    return ENOTSUP;
#else
		    /* 	Let the kernel handle this. */
		    return 0;
#endif
	}
	if (!fsess_isimpl(mp, FUSE_ACCESS)) {
		/* Let the kernel handle this. */
		    return 0;
	}
	if (dataflags & FSESS_DEFAULT_PERMISSIONS) {
		/* Let the kernel handle this. */
		    return 0;
	}
	if ((mode & VADMIN) != 0) {
		err = priv_check_cred(cred, PRIV_VFS_ADMIN);
		if (err) {
			return err;
		}
	}
	if ((mode & (VWRITE | VAPPEND | VADMIN)) != 0) {
		mask |= W_OK;
	}
	if ((mode & VREAD) != 0) {
		mask |= R_OK;
	}
	if ((mode & VEXEC) != 0) {
		mask |= X_OK;
	}
	bzero(&fdi, sizeof(fdi));

	fdisp_init(&fdi, sizeof(*fai));
	fdisp_make_vp(&fdi, FUSE_ACCESS, vp, td, cred);

	fai = fdi.indata;
	fai->mask = F_OK;
	fai->mask |= mask;

	err = fdisp_wait_answ(&fdi);
	fdisp_destroy(&fdi);

	if (err == ENOSYS) {
		fsess_set_notimpl(mp, FUSE_ACCESS);
		err = 0;
	}
	return err;
}

/*
 * Cache FUSE attributes from feo, in attr cache associated with vnode 'vp'.
 * Optionally, if argument 'vap' is not NULL, store a copy of the converted
 * attributes there as well.
 *
 * If the nominal attribute cache TTL is zero, do not cache on the 'vp' (but do
 * return the result to the caller).
 */
void
fuse_internal_cache_attrs(struct vnode *vp, struct fuse_attr *attr,
	uint64_t attr_valid, uint32_t attr_valid_nsec, struct vattr *vap)
{
	struct mount *mp;
	struct fuse_vnode_data *fvdat;
	struct vattr *vp_cache_at;

	mp = vnode_mount(vp);
	fvdat = VTOFUD(vp);

	/* Honor explicit do-not-cache requests from user filesystems. */
	if (attr_valid == 0 && attr_valid_nsec == 0)
		fvdat->valid_attr_cache = false;
	else
		fvdat->valid_attr_cache = true;

	vp_cache_at = VTOVA(vp);

	if (vap == NULL && vp_cache_at == NULL)
		return;

	if (vap == NULL)
		vap = vp_cache_at;

	vattr_null(vap);

	vap->va_fsid = mp->mnt_stat.f_fsid.val[0];
	vap->va_fileid = attr->ino;
	vap->va_mode = attr->mode & ~S_IFMT;
	vap->va_nlink     = attr->nlink;
	vap->va_uid       = attr->uid;
	vap->va_gid       = attr->gid;
	vap->va_rdev      = attr->rdev;
	vap->va_size      = attr->size;
	/* XXX on i386, seconds are truncated to 32 bits */
	vap->va_atime.tv_sec  = attr->atime;
	vap->va_atime.tv_nsec = attr->atimensec;
	vap->va_mtime.tv_sec  = attr->mtime;
	vap->va_mtime.tv_nsec = attr->mtimensec;
	vap->va_ctime.tv_sec  = attr->ctime;
	vap->va_ctime.tv_nsec = attr->ctimensec;
	vap->va_blocksize = PAGE_SIZE;
	vap->va_type = IFTOVT(attr->mode);
	vap->va_bytes = attr->blocks * S_BLKSIZE;
	vap->va_flags = 0;

	if (vap != vp_cache_at && vp_cache_at != NULL)
		memcpy(vp_cache_at, vap, sizeof(*vap));
}


/* fsync */

int
fuse_internal_fsync_callback(struct fuse_ticket *tick, struct uio *uio)
{
	if (tick->tk_aw_ohead.error == ENOSYS) {
		fsess_set_notimpl(tick->tk_data->mp, fticket_opcode(tick));
	}
	return 0;
}

int
fuse_internal_fsync(struct vnode *vp,
    struct thread *td,
    int waitfor,
    bool datasync)
{
	struct fuse_fsync_in *ffsi;
	struct fuse_dispatcher fdi;
	struct fuse_filehandle *fufh;
	struct fuse_vnode_data *fvdat = VTOFUD(vp);
	int op = FUSE_FSYNC;
	int type = 0;
	int err = 0;

	if (!fsess_isimpl(vnode_mount(vp),
	    (vnode_vtype(vp) == VDIR ? FUSE_FSYNCDIR : FUSE_FSYNC))) {
		return 0;
	}
	for (type = 0; type < FUFH_MAXTYPE; type++) {
		fufh = &(fvdat->fufh[type]);
		if (FUFH_IS_VALID(fufh)) {
			if (vnode_isdir(vp)) {
				op = FUSE_FSYNCDIR;
			}
			fdisp_init(&fdi, sizeof(*ffsi));
			fdisp_make_vp(&fdi, op, vp, td, NULL);
			ffsi = fdi.indata;
			ffsi->fh = fufh->fh_id;

			if (datasync)
				ffsi->fsync_flags = 1;

			if (waitfor == MNT_WAIT) {
				err = fdisp_wait_answ(&fdi);
			} else {
				fuse_insert_callback(fdi.tick,
					fuse_internal_fsync_callback);
				fuse_insert_message(fdi.tick);
			}
			fdisp_destroy(&fdi);
		}
	}

	return err;
}

/* readdir */

int
fuse_internal_readdir(struct vnode *vp,
    struct uio *uio,
    struct fuse_filehandle *fufh,
    struct fuse_iov *cookediov)
{
	int err = 0;
	struct fuse_dispatcher fdi;
	struct fuse_read_in *fri;

	if (uio_resid(uio) == 0) {
		return 0;
	}
	fdisp_init(&fdi, 0);

	/*
	 * Note that we DO NOT have a UIO_SYSSPACE here (so no need for p2p
	 * I/O).
	 */

	while (uio_resid(uio) > 0) {

		fdi.iosize = sizeof(*fri);
		fdisp_make_vp(&fdi, FUSE_READDIR, vp, NULL, NULL);

		fri = fdi.indata;
		fri->fh = fufh->fh_id;
		fri->offset = uio_offset(uio);
		fri->size = MIN(uio->uio_resid,
		    fuse_get_mpdata(vp->v_mount)->max_read);

		    if ((err = fdisp_wait_answ(&fdi))) {
			break;
		}
		if ((err = fuse_internal_readdir_processdata(uio, fri->size, fdi.answ,
		    fdi.iosize, cookediov))) {
			break;
		}
	}

	fdisp_destroy(&fdi);
	return ((err == -1) ? 0 : err);
}

int
fuse_internal_readdir_processdata(struct uio *uio,
    size_t reqsize,
    void *buf,
    size_t bufsize,
    void *param)
{
	int err = 0;
	int cou = 0;
	int bytesavail;
	size_t freclen;

	struct dirent *de;
	struct fuse_dirent *fudge;
	struct fuse_iov *cookediov = param;

	if (bufsize < FUSE_NAME_OFFSET) {
		return -1;
	}
	for (;;) {

		if (bufsize < FUSE_NAME_OFFSET) {
			err = -1;
			break;
		}
		fudge = (struct fuse_dirent *)buf;
		freclen = FUSE_DIRENT_SIZE(fudge);

		cou++;

		if (bufsize < freclen) {
			err = ((cou == 1) ? -1 : 0);
			break;
		}
#ifdef ZERO_PAD_INCOMPLETE_BUFS
		if (isbzero(buf, FUSE_NAME_OFFSET)) {
			err = -1;
			break;
		}
#endif

		if (!fudge->namelen || fudge->namelen > MAXNAMLEN) {
			err = EINVAL;
			break;
		}
		bytesavail = GENERIC_DIRSIZ((struct pseudo_dirent *)
					    &fudge->namelen);

		if (bytesavail > uio_resid(uio)) {
			err = -1;
			break;
		}
		fiov_refresh(cookediov);
		fiov_adjust(cookediov, bytesavail);

		de = (struct dirent *)cookediov->base;
		de->d_fileno = fudge->ino;
		de->d_reclen = bytesavail;
		de->d_type = fudge->type;
		de->d_namlen = fudge->namelen;
		memcpy((char *)cookediov->base + sizeof(struct dirent) - 
		       MAXNAMLEN - 1,
		       (char *)buf + FUSE_NAME_OFFSET, fudge->namelen);
		dirent_terminate(de);

		err = uiomove(cookediov->base, cookediov->len, uio);
		if (err) {
			break;
		}
		buf = (char *)buf + freclen;
		bufsize -= freclen;
		uio_setoffset(uio, fudge->off);
	}

	return err;
}

/* remove */

int
fuse_internal_remove(struct vnode *dvp,
    struct vnode *vp,
    struct componentname *cnp,
    enum fuse_opcode op)
{
	struct fuse_dispatcher fdi;
	struct fuse_vnode_data *fvdat;
	int err;

	err = 0;
	fvdat = VTOFUD(vp);

	fdisp_init(&fdi, cnp->cn_namelen + 1);
	fdisp_make_vp(&fdi, op, dvp, cnp->cn_thread, cnp->cn_cred);

	memcpy(fdi.indata, cnp->cn_nameptr, cnp->cn_namelen);
	((char *)fdi.indata)[cnp->cn_namelen] = '\0';

	err = fdisp_wait_answ(&fdi);
	fdisp_destroy(&fdi);
	return err;
}

/* rename */

int
fuse_internal_rename(struct vnode *fdvp,
    struct componentname *fcnp,
    struct vnode *tdvp,
    struct componentname *tcnp)
{
	struct fuse_dispatcher fdi;
	struct fuse_rename_in *fri;
	int err = 0;

	fdisp_init(&fdi, sizeof(*fri) + fcnp->cn_namelen + tcnp->cn_namelen + 2);
	fdisp_make_vp(&fdi, FUSE_RENAME, fdvp, tcnp->cn_thread, tcnp->cn_cred);

	fri = fdi.indata;
	fri->newdir = VTOI(tdvp);
	memcpy((char *)fdi.indata + sizeof(*fri), fcnp->cn_nameptr,
	    fcnp->cn_namelen);
	((char *)fdi.indata)[sizeof(*fri) + fcnp->cn_namelen] = '\0';
	memcpy((char *)fdi.indata + sizeof(*fri) + fcnp->cn_namelen + 1,
	    tcnp->cn_nameptr, tcnp->cn_namelen);
	((char *)fdi.indata)[sizeof(*fri) + fcnp->cn_namelen +
	    tcnp->cn_namelen + 1] = '\0';

	err = fdisp_wait_answ(&fdi);
	fdisp_destroy(&fdi);
	return err;
}

/* strategy */

/* entity creation */

void
fuse_internal_newentry_makerequest(struct mount *mp,
    uint64_t dnid,
    struct componentname *cnp,
    enum fuse_opcode op,
    void *buf,
    size_t bufsize,
    struct fuse_dispatcher *fdip)
{
	fdip->iosize = bufsize + cnp->cn_namelen + 1;

	fdisp_make(fdip, op, mp, dnid, cnp->cn_thread, cnp->cn_cred);
	memcpy(fdip->indata, buf, bufsize);
	memcpy((char *)fdip->indata + bufsize, cnp->cn_nameptr, cnp->cn_namelen);
	((char *)fdip->indata)[bufsize + cnp->cn_namelen] = '\0';
}

int
fuse_internal_newentry_core(struct vnode *dvp,
    struct vnode **vpp,
    struct componentname *cnp,
    enum vtype vtyp,
    struct fuse_dispatcher *fdip)
{
	int err = 0;
	struct fuse_entry_out *feo;
	struct mount *mp = vnode_mount(dvp);

	if ((err = fdisp_wait_answ(fdip))) {
		return err;
	}
	feo = fdip->answ;

	if ((err = fuse_internal_checkentry(feo, vtyp))) {
		return err;
	}
	err = fuse_vnode_get(mp, feo, feo->nodeid, dvp, vpp, cnp, vtyp);
	if (err) {
		fuse_internal_forget_send(mp, cnp->cn_thread, cnp->cn_cred,
		    feo->nodeid, 1);
		return err;
	}
	fuse_internal_cache_attrs(*vpp, &feo->attr, feo->attr_valid,
		feo->attr_valid_nsec, NULL);

	return err;
}

int
fuse_internal_newentry(struct vnode *dvp,
    struct vnode **vpp,
    struct componentname *cnp,
    enum fuse_opcode op,
    void *buf,
    size_t bufsize,
    enum vtype vtype)
{
	int err;
	struct fuse_dispatcher fdi;
	struct mount *mp = vnode_mount(dvp);

	fdisp_init(&fdi, 0);
	fuse_internal_newentry_makerequest(mp, VTOI(dvp), cnp, op, buf,
	    bufsize, &fdi);
	err = fuse_internal_newentry_core(dvp, vpp, cnp, vtype, &fdi);
	fdisp_destroy(&fdi);

	return err;
}

/* entity destruction */

int
fuse_internal_forget_callback(struct fuse_ticket *ftick, struct uio *uio)
{
	fuse_internal_forget_send(ftick->tk_data->mp, curthread, NULL,
	    ((struct fuse_in_header *)ftick->tk_ms_fiov.base)->nodeid, 1);

	return 0;
}

void
fuse_internal_forget_send(struct mount *mp,
    struct thread *td,
    struct ucred *cred,
    uint64_t nodeid,
    uint64_t nlookup)
{

	struct fuse_dispatcher fdi;
	struct fuse_forget_in *ffi;

	/*
         * KASSERT(nlookup > 0, ("zero-times forget for vp #%llu",
         *         (long long unsigned) nodeid));
         */

	fdisp_init(&fdi, sizeof(*ffi));
	fdisp_make(&fdi, FUSE_FORGET, mp, nodeid, td, cred);

	ffi = fdi.indata;
	ffi->nlookup = nlookup;

	fuse_insert_message(fdi.tick);
	fdisp_destroy(&fdi);
}

void
fuse_internal_vnode_disappear(struct vnode *vp)
{
	struct fuse_vnode_data *fvdat = VTOFUD(vp);

	ASSERT_VOP_ELOCKED(vp, "fuse_internal_vnode_disappear");
	fvdat->flag |= FN_REVOKED;
	fvdat->valid_attr_cache = false;
	cache_purge(vp);
}

/* fuse start/stop */

int
fuse_internal_init_callback(struct fuse_ticket *tick, struct uio *uio)
{
	int err = 0;
	struct fuse_data *data = tick->tk_data;
	struct fuse_init_out *fiio;

	if ((err = tick->tk_aw_ohead.error)) {
		goto out;
	}
	if ((err = fticket_pull(tick, uio))) {
		goto out;
	}
	fiio = fticket_resp(tick)->base;

	/* XXX: Do we want to check anything further besides this? */
	if (fiio->major < 7) {
		SDT_PROBE2(fuse, , internal, trace, 1,
			"userpace version too low");
		err = EPROTONOSUPPORT;
		goto out;
	}
	data->fuse_libabi_major = fiio->major;
	data->fuse_libabi_minor = fiio->minor;

	if (fuse_libabi_geq(data, 7, 5)) {
		if (fticket_resp(tick)->len == sizeof(struct fuse_init_out)) {
			data->max_write = fiio->max_write;
		} else {
			err = EINVAL;
		}
	} else {
		/* Old fix values */
		data->max_write = 4096;
	}

out:
	if (err) {
		fdata_set_dead(data);
	}
	FUSE_LOCK();
	data->dataflags |= FSESS_INITED;
	wakeup(&data->ticketer);
	FUSE_UNLOCK();

	return 0;
}

void
fuse_internal_send_init(struct fuse_data *data, struct thread *td)
{
	struct fuse_init_in *fiii;
	struct fuse_dispatcher fdi;

	fdisp_init(&fdi, sizeof(*fiii));
	fdisp_make(&fdi, FUSE_INIT, data->mp, 0, td, NULL);
	fiii = fdi.indata;
	fiii->major = FUSE_KERNEL_VERSION;
	fiii->minor = FUSE_KERNEL_MINOR_VERSION;
	/* 
	 * fusefs currently doesn't do any readahead other than fetching whole
	 * buffer cache block sized regions at once.  So the max readahead is
	 * the size of a buffer cache block.
	 */
	fiii->max_readahead = maxbcachebuf;
	fiii->flags = 0;

	fuse_insert_callback(fdi.tick, fuse_internal_init_callback);
	fuse_insert_message(fdi.tick);
	fdisp_destroy(&fdi);
}

#ifdef ZERO_PAD_INCOMPLETE_BUFS
static int
isbzero(void *buf, size_t len)
{
	int i;

	for (i = 0; i < len; i++) {
		if (((char *)buf)[i])
			return (0);
	}

	return (1);
}

#endif
