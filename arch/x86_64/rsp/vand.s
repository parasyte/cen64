//
// arch/x86_64/rsp/vand.s
//
// CEN64: Cycle-Accurate Nintendo 64 Simulator.
// Copyright (C) 2014, Tyler J. Stachecki.
//
// This file is subject to the terms and conditions defined in
// 'LICENSE', which is part of this source code package.
//

#
# xmm1 = vs
# xmm0 = vt
# xmm5 = acc_lo
#

.global RSP_VAND
.type	RSP_VAND, @function

RSP_VAND:
  pand %xmm1, %xmm0
  movdqa %xmm0, %xmm5
  retq

.size RSP_VAND,.-RSP_VAND

