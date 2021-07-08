/*
 * CPU frequency scaling support for Armada-8K platform.
 *
 * Copyright (C) 2016 Marvell
 *
 * Omri Itach <omrii@marvell.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt

#include <linux/err.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/platform_device.h>
#include <linux/clk.h>
#include <linux/cpu.h>
#include <linux/pm_opp.h>
#include <linux/cpufreq.h>

static int __init armada8k_cpufreq_driver_init(void)
{
	struct platform_device *pdev;
	struct device_node *node;
	int cpu;
	unsigned int cur_frequency;
	struct dev_pm_opp *opp;

	node = of_find_compatible_node(NULL, NULL, "marvell,ap806-cpu-clk");
	if (!node)
		node = of_find_compatible_node(NULL, NULL, "marvell,ap807-cpu-clk");

	if (!node || !of_device_is_available(node))
		return -ENODEV;

	/*
	 * For each CPU, this loop registers the operating points
	 * supported (which are the nominal CPU frequency and full integer
	 * divisions of it).
	 */
	for_each_possible_cpu(cpu) {
		struct device *cpu_dev;
		struct clk *clk;
		int ret;

		cpu_dev = get_cpu_device(cpu);
		if (!cpu_dev) {
			dev_err(cpu_dev, "Cannot get CPU %d\n", cpu);
			continue;
		}

		clk = clk_get(cpu_dev, 0);
		if (IS_ERR(clk)) {
			dev_err(cpu_dev, "Cannot get clock for CPU %d\n", cpu);
			return PTR_ERR(clk);
		}

		/* Get nominal (current) CPU frequency */
		cur_frequency = clk_get_rate(clk);
		if (!cur_frequency) {
			dev_err(cpu_dev, "Failed to get clock rate for CPU %d\n", cpu);
			return -EINVAL;
		}

		/* In case of a failure of dev_pm_opp_add(), we don't
		 * bother with cleaning up the registered OPP (there's
		 * no function to do so), and simply cancel the
		 * registration of the cpufreq device.
		 */
		ret = dev_pm_opp_add(cpu_dev, cur_frequency, 0);
		if (ret)
			return ret;

		ret = dev_pm_opp_add(cpu_dev, cur_frequency / 2, 0);
		if (ret)
			return ret;

		ret = dev_pm_opp_add(cpu_dev, cur_frequency / 3, 0);
		if (ret)
			return ret;
	}

	pdev = platform_device_register_simple("cpufreq-dt", -1, NULL, 0);

	/*
	 * If hot-unplugging both CPUs of cluster, and enabling back the 2nd
	 * CPU of cluster (before enabling the 1st CPU), then it doesn't register
	 * the static shared OPP entry from device tree.
	 * To overcome this (and to ensure 100Mhz will always be available) we test
	 * the existence of this entry per each CPU, and if missing, add it manually here.
	 *
	 * Note:
	 * Device tree must include OPPv2 table with at least one entry.
	 * If device tree describes a table with shared OPP entries
	 * ("opp-shared" marks DFS is shared between 2 CPUs of same cluster),
	 * then it's only added as shared, when the 1st CPU of each cluster is
	 * booted/plugged 1st (only then, the shared table is registered).
	 */
	for_each_possible_cpu(cpu) {
		struct device *cpu_dev;

		cpu_dev = get_cpu_device(cpu);
		if (!cpu_dev) {
			dev_err(cpu_dev, "Cannot get CPU %d\n", cpu);
			continue;
		}

		rcu_read_lock();
		opp = dev_pm_opp_find_freq_exact(cpu_dev, 100000000, true);
		rcu_read_unlock();

		if (IS_ERR(opp))
			dev_pm_opp_add(cpu_dev, 100000000, 0);
	}

	return PTR_ERR_OR_ZERO(pdev);
}
module_init(armada8k_cpufreq_driver_init);

MODULE_AUTHOR("Omri Itach <omrii@marvell.com>");
MODULE_DESCRIPTION("Armada 8K cpufreq driver");
MODULE_LICENSE("GPL");
