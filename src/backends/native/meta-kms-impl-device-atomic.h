/*
 * Copyright (C) 2019 Red Hat
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include "backends/native/meta-kms-impl-device.h"

#define META_TYPE_KMS_IMPL_DEVICE_ATOMIC (meta_kms_impl_device_atomic_get_type ())
META_EXPORT_TEST
G_DECLARE_FINAL_TYPE (MetaKmsImplDeviceAtomic, meta_kms_impl_device_atomic,
                      META, KMS_IMPL_DEVICE_ATOMIC, MetaKmsImplDevice)
