//
// vr4300/cp1.c: VR4300 floating point unit coprocessor.
//
// CEN64: Cycle-Accurate Nintendo 64 Simulator.
// Copyright (C) 2014, Tyler J. Stachecki.
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

#include "common.h"
#include "decoder.h"
#include "vr4300/cp1.h"
#include "vr4300/cpu.h"

static bool vr4300_cp1_usable(const struct vr4300 *vr4300);

//
// Determines if the coprocessor was used yet.
//
bool vr4300_cp1_usable(const struct vr4300 *vr4300) {
  return (vr4300->regs[VR4300_CP0_REGISTER_STATUS] & 0x20000000) != 0;
}

//
// ADD.fmt
//
int VR4300_CP1_ADD(struct vr4300 *vr4300, uint64_t fs, uint64_t ft) {
  struct vr4300_rfex_latch *rfex_latch = &vr4300->pipeline.rfex_latch;
  struct vr4300_exdc_latch *exdc_latch = &vr4300->pipeline.exdc_latch;

  uint32_t iw = rfex_latch->iw;
  unsigned dest = GET_RT(iw);
  enum vr4300_fmt fmt = GET_FMT(iw);

  uint64_t result;
  uint32_t fs32 = fs;
  uint32_t ft32 = ft;
  uint32_t fd32;

  if (fmt == VR4300_FMT_S) {
    __asm__ volatile(
      "flds %1\n\t"
      "flds %2\n\t"
      "faddp\n\t"
      "fstps %0\n\t"

      : "=m" (fd32)
      : "m" (fs32),
        "m" (ft32)
      : "st"
    );

    result = fd32;
  }

  else if (fmt == VR4300_FMT_D) {
    __asm__ volatile(
      "fldl %1\n\t"
      "fldl %2\n\t"
      "faddp\n\t"
      "fstpl %0\n\t"

      : "=m" (result)
      : "m" (fs),
        "m" (ft)
      : "st"
    );
  }

  else
    assert(0 && "Invalid instruction.");

  exdc_latch->result = result;
  exdc_latch->dest = dest;
  return 0;
}

//
// CFC1
//
int VR4300_CFC1(struct vr4300 *vr4300, uint64_t rs, uint64_t rt) {
  struct vr4300_rfex_latch *rfex_latch = &vr4300->pipeline.rfex_latch;
  struct vr4300_exdc_latch *exdc_latch = &vr4300->pipeline.exdc_latch;

  uint32_t iw = rfex_latch->iw;
  unsigned dest = GET_RT(iw);

  if (!vr4300_cp1_usable(vr4300)) {
    VR4300_CPU(vr4300);
    return 1;
  }

  exdc_latch->result = (int32_t) 0;
  exdc_latch->dest = dest;
  return 0;
}

//
// CTC1
//
int VR4300_CTC1(struct vr4300 *vr4300, uint64_t rs, uint64_t rt) {
  struct vr4300_rfex_latch *rfex_latch = &vr4300->pipeline.rfex_latch;

  if (!vr4300_cp1_usable(vr4300)) {
    VR4300_CPU(vr4300);
    return 1;
  }

  return 0;
}

//
// CVT.d.fmt
//
int VR4300_CP1_CVT_D(struct vr4300 *vr4300, uint64_t fs, uint64_t ft) {
  struct vr4300_rfex_latch *rfex_latch = &vr4300->pipeline.rfex_latch;
  struct vr4300_exdc_latch *exdc_latch = &vr4300->pipeline.exdc_latch;

  uint32_t iw = rfex_latch->iw;
  unsigned dest = GET_FD(iw);
  enum vr4300_fmt fmt = GET_FMT(iw);

  uint32_t fs32 = fs;
  uint64_t result;

  switch (fmt) {
    case VR4300_FMT_S:
      __asm__ volatile(
        "flds %1\n\t"
        "fstpl %0\n\t"

        : "=m" (result)
        : "m" (fs32)
        : "st"
      );

      break;

    case VR4300_FMT_D:
      assert(0 && "Invalid instruction.");
      break;

    case VR4300_FMT_W:
      __asm__ volatile(
        "fildl %1\n\t"
        "fstpl %0\n\t"

        : "=m" (result)
        : "m" (fs32)
        : "st"
      );

      break;

    case VR4300_FMT_L:
      __asm__ volatile(
        "fildq %1\n\t"
        "fstpl %0\n\t"

        : "=m" (result)
        : "m" (fs)
        : "st"
      );

      break;
  }

  exdc_latch->result = result;
  exdc_latch->dest = dest;
  return 0;
}

//
// CVT.l.fmt
//
int VR4300_CP1_CVT_L(struct vr4300 *vr4300, uint64_t fs, uint64_t ft) {
  struct vr4300_rfex_latch *rfex_latch = &vr4300->pipeline.rfex_latch;
  struct vr4300_exdc_latch *exdc_latch = &vr4300->pipeline.exdc_latch;

  uint32_t iw = rfex_latch->iw;
  unsigned dest = GET_FD(iw);
  enum vr4300_fmt fmt = GET_FMT(iw);

  uint32_t fs32 = fs;
  uint64_t result;

  switch (fmt) {
    case VR4300_FMT_D:
      __asm__ volatile(
        "fldl %1\n\t"
        "fistpq %0\n\t"

        : "=m" (result)
        : "m" (fs)
        : "st"
      );

      break;

    case VR4300_FMT_S:
      __asm__ volatile(
        "flds %1\n\t"
        "fistpq %0\n\t"

        : "=m" (result)
        : "m" (fs32)
        : "st"
      );

      break;

    case VR4300_FMT_L:
    case VR4300_FMT_W:
      assert(0 && "Invalid instruction.");
      break;
  }

  exdc_latch->result = result;
  exdc_latch->dest = dest;
  return 0;
}

//
// CVT.s.fmt
//
int VR4300_CP1_CVT_S(struct vr4300 *vr4300, uint64_t fs, uint64_t ft) {
  struct vr4300_rfex_latch *rfex_latch = &vr4300->pipeline.rfex_latch;
  struct vr4300_exdc_latch *exdc_latch = &vr4300->pipeline.exdc_latch;

  uint32_t iw = rfex_latch->iw;
  unsigned dest = GET_FD(iw);
  enum vr4300_fmt fmt = GET_FMT(iw);

  uint32_t fs32 = fs;
  uint32_t result;

  switch (fmt) {
    case VR4300_FMT_S:
      assert(0 && "Invalid instruction.");
      break;

    case VR4300_FMT_D:
      __asm__ volatile(
        "fldl %1\n\t"
        "fstps %0\n\t"

        : "=m" (result)
        : "m" (fs)
        : "st"
      );

      break;

    case VR4300_FMT_W:
      __asm__ volatile(
        "fildl %1\n\t"
        "fstps %0\n\t"

        : "=m" (result)
        : "m" (fs32)
        : "st"
      );

      break;

    case VR4300_FMT_L:
      __asm__ volatile(
        "fildq %1\n\t"
        "fstps %0\n\t"

        : "=m" (result)
        : "m" (fs)
        : "st"
      );

      break;
  }

  exdc_latch->result = result;
  exdc_latch->dest = dest;
  return 0;
}

//
// CVT.w.fmt
//
int VR4300_CP1_CVT_W(struct vr4300 *vr4300, uint64_t fs, uint64_t ft) {
  struct vr4300_rfex_latch *rfex_latch = &vr4300->pipeline.rfex_latch;
  struct vr4300_exdc_latch *exdc_latch = &vr4300->pipeline.exdc_latch;

  uint32_t iw = rfex_latch->iw;
  unsigned dest = GET_FD(iw);
  enum vr4300_fmt fmt = GET_FMT(iw);

  uint32_t fs32 = fs;
  uint32_t result;

  switch (fmt) {
    case VR4300_FMT_D:
      __asm__ volatile(
        "fldl %1\n\t"
        "fistpl %0\n\t"

        : "=m" (result)
        : "m" (fs)
        : "st"
      );

      break;

    case VR4300_FMT_S:
      __asm__ volatile(
        "flds %1\n\t"
        "fistpl %0\n\t"

        : "=m" (result)
        : "m" (fs32)
        : "st"
      );

      break;

    case VR4300_FMT_L:
    case VR4300_FMT_W:
      assert(0 && "Invalid instruction.");
      break;
  }

  exdc_latch->result = result;
  exdc_latch->dest = dest;
  return 0;
}

//
// DIV.fmt
//
int VR4300_CP1_DIV(struct vr4300 *vr4300, uint64_t fs, uint64_t ft) {
  struct vr4300_rfex_latch *rfex_latch = &vr4300->pipeline.rfex_latch;
  struct vr4300_exdc_latch *exdc_latch = &vr4300->pipeline.exdc_latch;

  uint32_t iw = rfex_latch->iw;
  unsigned dest = GET_RT(iw);
  enum vr4300_fmt fmt = GET_FMT(iw);

  uint64_t result;
  uint32_t fs32 = fs;
  uint32_t ft32 = ft;
  uint32_t fd32;

  if (fmt == VR4300_FMT_S) {
    __asm__ volatile(
      "flds %1\n\t"
      "flds %2\n\t"
      "fdivrp\n\t"
      "fstps %0\n\t"

      : "=m" (fd32)
      : "m" (fs32),
        "m" (ft32)
      : "st"
    );

    result = fd32;
  }

  else if (fmt == VR4300_FMT_D) {
    __asm__ volatile(
      "fldl %1\n\t"
      "fldl %2\n\t"
      "fdivrp\n\t"
      "fstpl %0\n\t"

      : "=m" (result)
      : "m" (fs),
        "m" (ft)
      : "st"
    );
  }

  else
    assert(0 && "Invalid instruction.");

  exdc_latch->result = result;
  exdc_latch->dest = dest;
  return 0;
}

//
// LDC1
//
// TODO/FIXME: Check for unaligned addresses.
//
int VR4300_LDC1(struct vr4300 *vr4300, uint64_t rs, uint64_t rt) {
  struct vr4300_rfex_latch *rfex_latch = &vr4300->pipeline.rfex_latch;
  struct vr4300_exdc_latch *exdc_latch = &vr4300->pipeline.exdc_latch;

  uint32_t iw = rfex_latch->iw;
  unsigned dest = GET_FT(iw);

  if (!vr4300_cp1_usable(vr4300)) {
    VR4300_CPU(vr4300);
    return 1;
  }

  exdc_latch->request.address = rs + (int16_t) iw;
  exdc_latch->request.dqm = ~0ULL;
  exdc_latch->request.postshift = 0;
  exdc_latch->request.preshift = 0;
  exdc_latch->request.type = VR4300_BUS_REQUEST_READ;
  exdc_latch->request.size = 8;

  exdc_latch->dest = dest;
  exdc_latch->result = 0;
  return 0;
}

//
// LWC1
//
// TODO/FIXME: Check for unaligned addresses.
//
int VR4300_LWC1(struct vr4300 *vr4300, uint64_t rs, uint64_t unused(rt)) {
  struct vr4300_rfex_latch *rfex_latch = &vr4300->pipeline.rfex_latch;
  struct vr4300_exdc_latch *exdc_latch = &vr4300->pipeline.exdc_latch;

  if (!vr4300_cp1_usable(vr4300)) {
    VR4300_CPU(vr4300);
    return 1;
  }

  uint32_t iw = rfex_latch->iw;
  uint64_t address = (rs + (int16_t) iw);
  uint32_t status = vr4300->regs[VR4300_CP0_REGISTER_STATUS];

  uint64_t result = 0;
  unsigned dest = GET_FT(iw);
  unsigned postshift = 0;

  if (!(status & 0x04000000)) {
    result = dest & 0x1
      ? vr4300->regs[dest & ~0x1] & 0x00000000FFFFFFFFULL
      : vr4300->regs[dest & ~0x1] & 0xFFFFFFFF00000000ULL;

    postshift = (dest & 0x1) << 5;
    dest &= ~0x1;
  }

  exdc_latch->request.address = address;
  exdc_latch->request.dqm = ~0U;
  exdc_latch->request.postshift = postshift;
  exdc_latch->request.preshift = 0;
  exdc_latch->request.type = VR4300_BUS_REQUEST_READ;
  exdc_latch->request.size = 4;

  exdc_latch->result = result;
  exdc_latch->dest = dest;
  return 0;
}

//
// MUL.fmt
//
int VR4300_CP1_MUL(struct vr4300 *vr4300, uint64_t fs, uint64_t ft) {
  struct vr4300_rfex_latch *rfex_latch = &vr4300->pipeline.rfex_latch;
  struct vr4300_exdc_latch *exdc_latch = &vr4300->pipeline.exdc_latch;

  uint32_t iw = rfex_latch->iw;
  unsigned dest = GET_RT(iw);
  enum vr4300_fmt fmt = GET_FMT(iw);

  uint64_t result;
  uint32_t fs32 = fs;
  uint32_t ft32 = ft;
  uint32_t fd32;

  if (fmt == VR4300_FMT_S) {
    __asm__ volatile(
      "flds %1\n\t"
      "flds %2\n\t"
      "fmulp\n\t"
      "fstps %0\n\t"

      : "=m" (fd32)
      : "m" (fs32),
        "m" (ft32)
      : "st"
    );

    result = fd32;
  }

  else if (fmt == VR4300_FMT_D) {
    __asm__ volatile(
      "fldl %1\n\t"
      "fldl %2\n\t"
      "fmulp\n\t"
      "fstpl %0\n\t"

      : "=m" (result)
      : "m" (fs),
        "m" (ft)
      : "st"
    );
  }

  else
    assert(0 && "Invalid instruction.");

  exdc_latch->result = result;
  exdc_latch->dest = dest;
  return 0;
}

//
// MFC1
//
int VR4300_MFC1(struct vr4300 *vr4300, uint64_t fs, uint64_t unused(rt)) {
  struct vr4300_rfex_latch *rfex_latch = &vr4300->pipeline.rfex_latch;
  struct vr4300_exdc_latch *exdc_latch = &vr4300->pipeline.exdc_latch;
  uint32_t status = vr4300->regs[VR4300_CP0_REGISTER_STATUS];

  uint64_t result;
  uint32_t iw = rfex_latch->iw;
  unsigned fs_reg = GET_FS(iw);
  unsigned dest = GET_RT(iw);

  // TODO/FIXME: XXX
  assert(!(fs_reg & 0x1));
  result = (int32_t) fs;

  exdc_latch->result = result;
  exdc_latch->dest = dest;
  return 0;
}

//
// MTC1
//
int VR4300_MTC1(struct vr4300 *vr4300, uint64_t fs, uint64_t rt) {
  struct vr4300_rfex_latch *rfex_latch = &vr4300->pipeline.rfex_latch;
  struct vr4300_exdc_latch *exdc_latch = &vr4300->pipeline.exdc_latch;
  uint32_t status = vr4300->regs[VR4300_CP0_REGISTER_STATUS];

  uint64_t result = (int32_t) rt;
  uint32_t iw = rfex_latch->iw;
  unsigned dest = GET_FS(iw);

  if (!(status & 0x04000000)) {
    result = (dest & 0x1)
      ? ((uint32_t) fs) | (rt << 32)
      : (fs << 32) | ((uint32_t) rt);

    dest &= ~0x1;
  }

  exdc_latch->result = result;
  exdc_latch->dest = dest;
  return 0;
}

//
// SDC1
//
// TODO/FIXME: Check for unaligned addresses.
//
int VR4300_SDC1(struct vr4300 *vr4300, uint64_t rs, uint64_t ft) {
  struct vr4300_rfex_latch *rfex_latch = &vr4300->pipeline.rfex_latch;
  struct vr4300_exdc_latch *exdc_latch = &vr4300->pipeline.exdc_latch;

  uint32_t iw = rfex_latch->iw;

  if (!vr4300_cp1_usable(vr4300)) {
    VR4300_CPU(vr4300);
    return 1;
  }

  exdc_latch->request.address = rs + (int16_t) iw;
  exdc_latch->request.data = ft;
  exdc_latch->request.dqm = ~0ULL;
  exdc_latch->request.postshift = 0;
  exdc_latch->request.preshift = 0;
  exdc_latch->request.type = VR4300_BUS_REQUEST_WRITE;
  exdc_latch->request.size = 8;

  exdc_latch->result = 0;
  return 0;
}

//
// SWC1
//
// TODO/FIXME: Check for unaligned addresses.
//
int VR4300_SWC1(struct vr4300 *vr4300, uint64_t rs, uint64_t ft) {
  struct vr4300_rfex_latch *rfex_latch = &vr4300->pipeline.rfex_latch;
  struct vr4300_exdc_latch *exdc_latch = &vr4300->pipeline.exdc_latch;
  uint32_t status = vr4300->regs[VR4300_CP0_REGISTER_STATUS];

  uint32_t iw = rfex_latch->iw;
  unsigned ft_reg = GET_FT(iw);

  if (!vr4300_cp1_usable(vr4300)) {
    VR4300_CPU(vr4300);
    return 1;
  }

  if (!(status & 0x04000000))
    ft >>= ((ft_reg & 0x1) << 5);

  exdc_latch->request.address = rs + (int16_t) iw;
  exdc_latch->request.data = ft;
  exdc_latch->request.dqm = ~0U;
  exdc_latch->request.postshift = 0;
  exdc_latch->request.preshift = 0;
  exdc_latch->request.type = VR4300_BUS_REQUEST_WRITE;
  exdc_latch->request.size = 4;

  exdc_latch->result = 0;
  return 0;
}

//
// TRUNC.w.fmt
//
int VR4300_CP1_TRUNC_W(struct vr4300 *vr4300, uint64_t fs, uint64_t ft) {
  struct vr4300_rfex_latch *rfex_latch = &vr4300->pipeline.rfex_latch;
  struct vr4300_exdc_latch *exdc_latch = &vr4300->pipeline.exdc_latch;

  uint32_t iw = rfex_latch->iw;
  unsigned dest = GET_FD(iw);
  enum vr4300_fmt fmt = GET_FMT(iw);

  uint32_t fs32 = fs;
  uint32_t result;

  switch (fmt) {
    case VR4300_FMT_S:
      __asm__ volatile(
        "flds %1\n\t"
        "fisttpl %0\n\t"

        : "=m" (result)
        : "m" (fs32)
        : "st"
      );

      break;

    case VR4300_FMT_D:
      __asm__ volatile(
        "fldl %1\n\t"
        "fisttpl %0\n\t"

        : "=m" (result)
        : "m" (fs)
        : "st"
      );

      break;

    case VR4300_FMT_W:
    case VR4300_FMT_L:
      assert(0 && "Invalid instruction.");
      break;
  }

  exdc_latch->result = result;
  exdc_latch->dest = dest;
  return 0;
}

// Initializes the coprocessor.
void vr4300_cp1_init(struct vr4300 *vr4300) {
}

