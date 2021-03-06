/*
 * Code borrowed from powerpc/kernel/pci-common.c
 *
 * Copyright (C) 2003 Anton Blanchard <anton@au.ibm.com>, IBM
 * Copyright (C) 2014 ARM Ltd.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * version 2 as published by the Free Software Foundation.
 *
 */

#include <linux/acpi.h>
#include <linux/init.h>
#include <linux/io.h>
#include <linux/kernel.h>
#include <linux/mm.h>
#include <linux/of_pci.h>
#include <linux/of_platform.h>
#include <linux/of_irq.h>
#include <linux/pcieport_if.h>
#include <linux/slab.h>

#include <asm/pci-bridge.h>

/*
 * Called after each bus is probed, but before its children are examined
 */
void pcibios_fixup_bus(struct pci_bus *bus)
{
	/* nothing to do, expected to be removed in the future */
}

/*
 * We don't have to worry about legacy ISA devices, so nothing to do here
 */
resource_size_t pcibios_align_resource(void *data, const struct resource *res,
				resource_size_t size, resource_size_t align)
{
	return res->start;
}

/**
 * pcibios_enable_device - Enable I/O and memory.
 * @dev: PCI device to be enabled
 * @mask: bitmask of BARs to enable
 */
int pcibios_enable_device(struct pci_dev *dev, int mask)
{
	if (pci_has_flag(PCI_PROBE_ONLY))
		return 0;

	return pci_enable_resources(dev, mask);
}

/*
 * Try to assign the IRQ number from DT when adding a new device
 */
int pcibios_add_device(struct pci_dev *dev)
{
	dev->irq = of_irq_parse_and_map_pci(dev, 0, 0);

	return 0;
}

int pci_mmap_page_range(struct pci_dev *dev, struct vm_area_struct *vma,
			enum pci_mmap_state mmap_state, int write_combine)
{
	if (mmap_state == pci_mmap_io)
		return -EINVAL;

	/*
	 * Mark this as IO
	 */
	vma->vm_page_prot = pgprot_noncached(vma->vm_page_prot);

	if (remap_pfn_range(vma, vma->vm_start, vma->vm_pgoff,
			     vma->vm_end - vma->vm_start,
			     vma->vm_page_prot))
		return -EAGAIN;

	return 0;
}

/*
 * Check device tree if the service interrupts are there
 */
int pcibios_check_service_irqs(struct pci_dev *dev, int *irqs, int mask)
{
	int ret, count = 0;
	struct device_node *np = NULL;

	if (dev->bus->dev.of_node)
		np = dev->bus->dev.of_node;

	if (np == NULL)
		return 0;

	if (!IS_ENABLED(CONFIG_OF_IRQ))
		return 0;

	/* If root port doesn't support MSI/MSI-X/INTx in RC mode,
	 * request irq for aer
	 */
	if (mask & PCIE_PORT_SERVICE_AER) {
		ret = of_irq_get_byname(np, "aer");
		if (ret > 0) {
			irqs[PCIE_PORT_SERVICE_AER_SHIFT] = ret;
			count++;
		}
	}

	if (mask & PCIE_PORT_SERVICE_PME) {
		ret = of_irq_get_byname(np, "pme");
		if (ret > 0) {
			irqs[PCIE_PORT_SERVICE_PME_SHIFT] = ret;
			count++;
		}
	}

	/* TODO: add more service interrupts if there it is in the device tree*/

	return count;
}

/*
 * raw_pci_read/write - Platform-specific PCI config space access.
 */
int raw_pci_read(unsigned int domain, unsigned int bus,
		  unsigned int devfn, int reg, int len, u32 *val)
{
	return -ENXIO;
}

int raw_pci_write(unsigned int domain, unsigned int bus,
		unsigned int devfn, int reg, int len, u32 val)
{
	return -ENXIO;
}

#ifdef CONFIG_ACPI
/* Root bridge scanning */
struct pci_bus *pci_acpi_scan_root(struct acpi_pci_root *root)
{
	/* TODO: Should be revisited when implementing PCI on ACPI */
	return NULL;
}
#endif
