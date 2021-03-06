/* SPDX-License-Identifier: LGPL-2.1+ */
#pragma once

/***
  This file is part of systemd.

  Copyright 2010 Lennart Poettering
***/

#include "manager.h"

extern const sd_bus_vtable bus_manager_vtable[];

void bus_manager_send_finished(Manager *m, usec_t firmware_usec, usec_t loader_usec, usec_t kernel_usec, usec_t initrd_usec, usec_t userspace_usec, usec_t total_usec);
void bus_manager_send_reloading(Manager *m, bool active);
void bus_manager_send_change_signal(Manager *m);

int verify_run_space_and_log(const char *message);
