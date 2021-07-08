/*
 * Marvell Armada AP806 CPU Clock Controller
 *
 * Copyright (C) 2016 Marvell
 *
 * Omri Itach <omrii@marvell.com>
 *
 * This file is licensed under the terms of the GNU General Public
 * License version 2.  This program is licensed "as is" without any
 * warranty of any kind, whether express or implied.
 */

#include <linux/clk-provider.h>
#include <linux/clk.h>
#include <linux/err.h>
#include <linux/kernel.h>
#include <linux/mfd/syscon.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/regmap.h>
#include <linux/delay.h>

/* Stub clocks id */
#define AP806_CPU_CLUSTER0		0
#define AP806_CPU_CLUSTER1		1
#define AP806_CPUS_PER_CLUSTER		2
#define APN806_CPU1_MASK		0x1

#define APN806_CLUSTER_NUM_OFFSET	8
#define APN806_CLUSTER_NUM_MASK		(1 << APN806_CLUSTER_NUM_OFFSET)


#define KHZ_TO_HZ			1000

/*
 * struct cpu_dfs_regs: CPU DFS register mapping
 * @divider_reg: Full Integer Ratio from PLL-out frequency to CPU clock frequency
 * @force_reg: Request to force new ratio regardless of relation to other clocks
 * @ratio_reg: Central request to switch ratios
 * @divider_ratio: cpu divider has two channels with ratio
 */
struct cpu_dfs_regs {
	unsigned int divider_reg;
	unsigned int force_reg;
	unsigned int ratio_reg;
	unsigned int divider_mask;
	unsigned int cluster_offset;
	unsigned int force_mask;
	int divider_offset;
	int divider_ratio;
	int ratio_offset;
};

/* AP806 CPU DFS register mapping*/
#define AP806_CA72MP2_0_PLL_CR_0_REG_OFFSET		0x78
#define AP806_CA72MP2_0_PLL_CR_1_REG_OFFSET		0x80
#define AP806_CA72MP2_0_PLL_CR_2_REG_OFFSET		0x84

#define AP806_CA72MP2_0_PLL_CR_CLUSTER_OFFSET		0x14
#define AP806_PLL_CR_0_CPU_CLK_DIV_RATIO_OFFSET		0
#define AP806_PLL_CR_CPU_CLK_DIV_RATIO			0
#define AP806_PLL_CR_0_CPU_CLK_DIV_RATIO_MASK		(0x3f << AP806_PLL_CR_0_CPU_CLK_DIV_RATIO_OFFSET)
#define AP806_PLL_CR_0_CPU_CLK_RELOAD_FORCE_OFFSET	24
#define AP806_PLL_CR_0_CPU_CLK_RELOAD_FORCE_MASK	(0x1 << AP806_PLL_CR_0_CPU_CLK_RELOAD_FORCE_OFFSET)
#define AP806_PLL_CR_0_CPU_CLK_RELOAD_RATIO_OFFSET	16

/* AP807 CPU DFS register mapping*/
#define AP807_CA72MP2_0_PLL_CR_0_REG_OFFSET		0x78
#define AP807_CA72MP2_0_PLL_CR_1_REG_OFFSET		0x7c
#define AP807_CA72MP2_0_PLL_CR_2_REG_OFFSET		0x7c

#define AP807_CA72MP2_0_PLL_CR_CLUSTER_OFFSET		0x8
#define AP807_PLL_CR_0_CPU_CLK_DIV_RATIO_OFFSET		18
#define AP807_PLL_CR_0_CPU_CLK_DIV_RATIO_MASK		(0x3f << AP807_PLL_CR_0_CPU_CLK_DIV_RATIO_OFFSET)
#define AP807_PLL_CR_1_CPU_CLK_DIV_RATIO_OFFSET         12
#define AP807_PLL_CR_1_CPU_CLK_DIV_RATIO_MASK		(0x3f << AP807_PLL_CR_1_CPU_CLK_DIV_RATIO_OFFSET)
#define AP807_PLL_CR_CPU_CLK_DIV_RATIO			3
#define AP807_PLL_CR_0_CPU_CLK_RELOAD_FORCE_OFFSET	0
#define AP807_PLL_CR_0_CPU_CLK_RELOAD_FORCE_MASK	(0x3 << AP807_PLL_CR_0_CPU_CLK_RELOAD_FORCE_OFFSET)
#define AP807_PLL_CR_0_CPU_CLK_RELOAD_RATIO_OFFSET	6

#define to_clk(hw) container_of(hw, struct ap806_clk, hw)

/* AP806 CPU DFS register mapping */
struct cpu_dfs_regs ap806_dfs_regs = {
	AP806_CA72MP2_0_PLL_CR_0_REG_OFFSET,
	AP806_CA72MP2_0_PLL_CR_1_REG_OFFSET,
	AP806_CA72MP2_0_PLL_CR_2_REG_OFFSET,
	AP806_PLL_CR_0_CPU_CLK_DIV_RATIO_MASK,
	AP806_CA72MP2_0_PLL_CR_CLUSTER_OFFSET,
	AP806_PLL_CR_0_CPU_CLK_RELOAD_FORCE_MASK,
	AP806_PLL_CR_0_CPU_CLK_DIV_RATIO_OFFSET,
	AP806_PLL_CR_CPU_CLK_DIV_RATIO,
	AP806_PLL_CR_0_CPU_CLK_RELOAD_RATIO_OFFSET
};

/* AP807 CPU DFS register mapping */
struct cpu_dfs_regs ap807_dfs_regs = {
	AP807_CA72MP2_0_PLL_CR_0_REG_OFFSET,
	AP807_CA72MP2_0_PLL_CR_1_REG_OFFSET,
	AP807_CA72MP2_0_PLL_CR_2_REG_OFFSET,
	AP807_PLL_CR_0_CPU_CLK_DIV_RATIO_MASK,
	AP807_CA72MP2_0_PLL_CR_CLUSTER_OFFSET,
	AP807_PLL_CR_0_CPU_CLK_RELOAD_FORCE_MASK,
	AP807_PLL_CR_0_CPU_CLK_DIV_RATIO_OFFSET,
	AP807_PLL_CR_CPU_CLK_DIV_RATIO,
	AP807_PLL_CR_0_CPU_CLK_RELOAD_RATIO_OFFSET
};
/*
 * struct ap806_clk: CPU cluster clock controller instance
 * @max_cpu_freq: Max CPU frequency - Sample at rest boot configuration
 * @cluster: Cluster clock controller index
 * @clk_name: Cluster clock controller name
 * @parent_name: Cluster clock controller parent name
 * @hw: HW specific structure of Cluster clock controller
 * @pll_regs: CPU DFS register mapping
 * @pll_cr_base: CA72MP2 Register base (Device Sample at Reset register)
 */
struct ap806_clk {
	int max_cpu_freq;
	int cluster;

	const char *clk_name;
	const char *parent_name;

	struct device *dev;
	struct clk_hw hw;

	struct regmap *pll_cr_base;

	struct cpu_dfs_regs *pll_regs;
};

static struct clk **cluster_clks;
static struct clk_onecell_data clk_data;

static unsigned long ap806_clk_recalc_rate(struct clk_hw *hw, unsigned long parent_rate)
{
	unsigned long rate = 0;
	struct ap806_clk *clk = to_clk(hw);
	int cpu_clkdiv_ratio;

	unsigned int cpu_clkdiv_reg = clk->pll_regs->divider_reg +
		(clk->cluster * clk->pll_regs->cluster_offset);

	/* AP806 supports 2 Clusters */
	if (clk->cluster != AP806_CPU_CLUSTER0 && clk->cluster != AP806_CPU_CLUSTER1) {
		dev_err(clk->dev, "%s: un-supported clock cluster id %d\n",
			__func__, clk->cluster);
		return -EINVAL;
	}

	regmap_read(clk->pll_cr_base, cpu_clkdiv_reg, &cpu_clkdiv_ratio);
	rate = clk->max_cpu_freq / ((cpu_clkdiv_ratio & clk->pll_regs->divider_mask)
			>> clk->pll_regs->divider_offset);
	rate *= KHZ_TO_HZ;	/* convert from kHz to Hz */

	return rate;
}

static int ap806_clk_set_rate(struct clk_hw *hw, unsigned long rate,
			      unsigned long parent_rate)
{
	struct ap806_clk *clk = to_clk(hw);
	unsigned long new_rate = rate / KHZ_TO_HZ;  /* kHz */
	int reg, divider = clk->max_cpu_freq / new_rate;
	unsigned int cpu_clkdiv_reg, cpu_force_reg, cpu_ratio_reg;

	cpu_clkdiv_reg = clk->pll_regs->divider_reg +
		(clk->cluster * clk->pll_regs->cluster_offset);
	cpu_force_reg = clk->pll_regs->force_reg +
		(clk->cluster * clk->pll_regs->cluster_offset);
	cpu_ratio_reg = clk->pll_regs->ratio_reg +
		(clk->cluster * clk->pll_regs->cluster_offset);

	/* AP806 supports 2 Clusters */
	if (clk->cluster != AP806_CPU_CLUSTER0 && clk->cluster != AP806_CPU_CLUSTER1) {
		dev_err(clk->dev, "%s: un-supported clock cluster id %d\n",
			__func__, clk->cluster);
		return -EINVAL;
	}

	/* 1. Set CPU divider */
	regmap_read(clk->pll_cr_base, cpu_clkdiv_reg, &reg);
	reg &= ~(clk->pll_regs->divider_mask);
	reg |= (divider << clk->pll_regs->divider_offset);

	/* AP807 cpu divider has two channels with ratio 1:3 */
	if (clk->pll_regs->divider_ratio) {
		reg &= ~(AP807_PLL_CR_1_CPU_CLK_DIV_RATIO_MASK);
		reg |= ((divider * clk->pll_regs->divider_ratio) <<
				AP807_PLL_CR_1_CPU_CLK_DIV_RATIO_OFFSET);
	}
	regmap_write(clk->pll_cr_base, cpu_clkdiv_reg, reg);

	/* 2. Set Reload force */
	regmap_read(clk->pll_cr_base, cpu_force_reg, &reg);
	reg |= clk->pll_regs->force_mask;
	regmap_write(clk->pll_cr_base, cpu_force_reg, reg);

	/* 3. Set Reload ratio */
	regmap_read(clk->pll_cr_base, cpu_ratio_reg, &reg);
	reg |= 0x1 << clk->pll_regs->ratio_offset;
	regmap_write(clk->pll_cr_base, cpu_ratio_reg, reg);

	/* 4. Wait for stabilizing CPU Clock */
	ndelay(100);

	/* 5. Clear Reload ratio */
	regmap_read(clk->pll_cr_base, cpu_ratio_reg, &reg);
	reg &= ~(0x1 << clk->pll_regs->ratio_offset);
	regmap_write(clk->pll_cr_base, cpu_ratio_reg, reg);

	return 0;
}

static long ap806_clk_round_rate(struct clk_hw *hw, unsigned long rate,
				 unsigned long *parent_rate)
{
	struct ap806_clk *clk = to_clk(hw);
	unsigned long new_rate = rate / KHZ_TO_HZ;  /* kHz */
	int divider = clk->max_cpu_freq / new_rate;

	/* AP806 supports 2 Clusters */
	if (clk->cluster != AP806_CPU_CLUSTER0 && clk->cluster != AP806_CPU_CLUSTER1) {
		dev_err(clk->dev, "%s: un-supported clock cluster id %d\n",
			__func__, clk->cluster);
		return -EINVAL;
	}

	new_rate = (clk->max_cpu_freq / divider) * KHZ_TO_HZ; /* convert kHz to Hz */
	return new_rate;
}

static const struct clk_ops ap806_clk_ops = {
	.recalc_rate	= ap806_clk_recalc_rate,
	.round_rate	= ap806_clk_round_rate,
	.set_rate	= ap806_clk_set_rate,
};

static const struct of_device_id ap806_clk_of_match[] = {
	{ .compatible = "marvell,ap806-cpu-clk", },
	{ .compatible = "marvell,ap807-cpu-clk", },
	{}
};

static int ap806_clk_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct ap806_clk *ap806_clk;
	struct clk *max_cpu_clk;
	struct device_node *dn, *np = pdev->dev.of_node;
	int ret, nclusters = 0, cluster_index = 0;
	struct regmap *reg;

	/* set initial CPU frequency as maximum possible frequency */
	max_cpu_clk = devm_clk_get(&pdev->dev, NULL);
	if (IS_ERR(max_cpu_clk)) {
		dev_err(dev, "error getting max cpu frequency\n");
		return PTR_ERR(max_cpu_clk);
	}

	reg = syscon_node_to_regmap(np);
	if (IS_ERR(reg)) {
		pr_err("cannot get pll_cr_base regmap\n");
		return PTR_ERR(reg);
	}

	/*
	 * AP806 has 4 cpus and DFS for AP806 is controlled per cluster
	 * (2 CPUs per cluster), cpu0 and cpu1 are fixed to cluster0 while
	 * cpu2 and cpu3 are fixed to cluster1 whether they are enabled or not.
	 * Since cpu0 is the boot cpu, then cluster0 must exist.
	 * If cpu2 or cpu3 is enabled, cluster1 will exist and the cluster
	 * number is 2; otherwise the cluster number is 1.
	 */
	nclusters = 1;
	for_each_node_by_type(dn, "cpu") {
		int cpu, err;

		err = of_property_read_u32(dn, "reg", &cpu);
		if (WARN_ON(err))
			return err;

		/* If cpu2 or cpu3 is enabled */
		if ((cpu & APN806_CLUSTER_NUM_MASK)) {
			nclusters = 2;
			break;
		}
	}

	/* DFS for AP806 is controlled per cluster (2 CPUs per cluster),
	 * so allocate structs per cluster
	 */
	ap806_clk = devm_kzalloc(dev, nclusters * sizeof(*ap806_clk), GFP_KERNEL);
	if (WARN_ON(!ap806_clk))
		return -ENOMEM;

	cluster_clks = devm_kzalloc(dev, nclusters * sizeof(*cluster_clks), GFP_KERNEL);
	if (WARN_ON(!cluster_clks))
		return -ENOMEM;

	for_each_node_by_type(dn, "cpu") {
		struct clk_init_data init;
		struct clk *clk;
		char *clk_name = devm_kzalloc(dev, 9, GFP_KERNEL);
		int cpu, err;

		if (WARN_ON(!clk_name))
			return -ENOMEM;

		err = of_property_read_u32(dn, "reg", &cpu);
		if (WARN_ON(err))
			return err;

		cluster_index = (cpu & APN806_CLUSTER_NUM_MASK) >> APN806_CLUSTER_NUM_OFFSET;

		/* Initialize once for one cluster */
		if (cluster_clks[cluster_index])
			continue;

		sprintf(clk_name, "cluster%d", cluster_index);
		ap806_clk[cluster_index].parent_name = of_clk_get_parent_name(np, 0);
		ap806_clk[cluster_index].clk_name = clk_name;
		ap806_clk[cluster_index].cluster = cluster_index;
		ap806_clk[cluster_index].pll_cr_base = reg;
		ap806_clk[cluster_index].max_cpu_freq = clk_get_rate(max_cpu_clk) / 1000; /* Mhz */
		ap806_clk[cluster_index].hw.init = &init;
		ap806_clk[cluster_index].dev = dev;

		if (of_device_is_compatible(pdev->dev.of_node,
					"marvell,ap806-cpu-clk"))
			ap806_clk[cluster_index].pll_regs = &ap806_dfs_regs;
		else if (of_device_is_compatible(pdev->dev.of_node,
					"marvell,ap807-cpu-clk"))
			ap806_clk[cluster_index].pll_regs = &ap807_dfs_regs;

		init.name = ap806_clk[cluster_index].clk_name;
		init.ops = &ap806_clk_ops;
		init.num_parents = 0;
		init.flags = CLK_GET_RATE_NOCACHE | CLK_IS_ROOT;

		clk = devm_clk_register(dev, &ap806_clk[cluster_index].hw);
		if (IS_ERR(clk))
			return PTR_ERR(clk);

		cluster_clks[cluster_index] = clk;
	}

	clk_data.clk_num = cluster_index + 1;
	clk_data.clks = cluster_clks;

	ret = of_clk_add_provider(np, of_clk_src_onecell_get, &clk_data);
	if (ret) {
		dev_err(dev, "failed to register OF clock provider\n");
		return ret;
	}

	return 0;
}

static struct platform_driver ap806_clk_driver = {
	.driver	= {
		.name = "ap806-clk",
		.of_match_table = ap806_clk_of_match,
	},
	.probe = ap806_clk_probe,
};

static int __init ap806_clk_init(void)
{
	return platform_driver_register(&ap806_clk_driver);
}
subsys_initcall(ap806_clk_init);

