/*
 * Copyright (C) 2016 Marvell
 *
 * Antoine Tenart <antoine.tenart@free-electrons.com>
 *
 * This file is licensed under the terms of the GNU General Public
 * License version 2. This program is licensed "as is" without any
 * warranty of any kind, whether express or implied.
 */

#include <linux/dma-mapping.h>
#include <linux/spinlock.h>

#include "safexcel.h"

/* Initialize the ring descriptors */
int safexcel_init_ring_descriptors(struct safexcel_crypto_priv *priv,
				   struct safexcel_desc_ring *cdr,
				   struct safexcel_desc_ring *rdr)
{
	cdr->offset = sizeof(u32) * priv->config.cd_offset;
	cdr->base = dmam_alloc_coherent(priv->dev,
					cdr->offset * EIP197_DEFAULT_RING_SIZE,
					&cdr->base_dma, GFP_KERNEL);
	if (!cdr->base)
		return -ENOMEM;
	cdr->write = cdr->base;
	cdr->base_end = cdr->base + cdr->offset * (EIP197_DEFAULT_RING_SIZE - 1);
	cdr->read = cdr->base;

	rdr->offset = sizeof(u32) * priv->config.rd_offset;
	rdr->base = dmam_alloc_coherent(priv->dev,
					rdr->offset * EIP197_DEFAULT_RING_SIZE,
					&rdr->base_dma, GFP_KERNEL);
	if (!rdr->base)
		return -ENOMEM;

	rdr->write = rdr->base;
	rdr->base_end = rdr->base + rdr->offset  * (EIP197_DEFAULT_RING_SIZE - 1);
	rdr->read = rdr->base;

	return 0;
}

/* Return the next available descriptor for use (command/result) */
static void *safexcel_ring_next_wptr(struct safexcel_crypto_priv *priv,
				     struct safexcel_desc_ring *ring)
{
	void *ptr = ring->write;

	if ((ring->write == ring->read - ring->offset) ||
	    (ring->read == ring->base && ring->write == ring->base_end))
		return ERR_PTR(-ENOMEM);

	ring->write += ring->offset;
	if (ring->write > ring->base_end)
		ring->write = ring->base;

	return ptr;
}

/* Return the last used descriptor (command/result) */
void *safexcel_ring_next_rptr(struct safexcel_crypto_priv *priv,
			      struct safexcel_desc_ring *ring)
{
	void *ptr = ring->read;

	if (ring->write == ring->read)
		return ERR_PTR(-ENOENT);

	ring->read += ring->offset;
	if (ring->read > ring->base_end)
		ring->read = ring->base;

	return ptr;
}

inline void *safexcel_ring_curr_rptr(struct safexcel_crypto_priv *priv,
				     int ring)
{
	struct safexcel_desc_ring *rdr = &priv->ring[ring].rdr;

	return rdr->read;
}

inline int safexcel_ring_first_rdr_index(struct safexcel_crypto_priv *priv,
					 int ring)
{
	struct safexcel_desc_ring *rdr = &priv->ring[ring].rdr;

	return (rdr->read - rdr->base) / rdr->offset;
}

inline int safexcel_ring_rdr_rdesc_index(struct safexcel_crypto_priv *priv,
					 int ring,
					 struct safexcel_result_desc *rdesc)
{
	struct safexcel_desc_ring *rdr = &priv->ring[ring].rdr;

	return ((void *)rdesc - rdr->base) / rdr->offset;
}

/* Rollback descriptor allocation (in a case of insufficient resources) */
void safexcel_ring_rollback_wptr(struct safexcel_crypto_priv *priv,
				 struct safexcel_desc_ring *ring)
{
	if (ring->write == ring->read)
		return;

	if (ring->write == ring->base)
		ring->write = ring->base_end;
	else
		ring->write -= ring->offset;
}

/* Create a command descriptor */
struct safexcel_command_desc *safexcel_add_cdesc(struct safexcel_crypto_priv *priv,
						 int ring_id,
						 bool first, bool last,
						 dma_addr_t data, u32 data_len,
						 u32 full_data_len,
						 dma_addr_t context) {
	struct safexcel_command_desc *cdesc;
	int i;

	cdesc = safexcel_ring_next_wptr(priv, &priv->ring[ring_id].cdr);
	if (IS_ERR(cdesc))
		return cdesc;

	memset(cdesc, 0, sizeof(struct safexcel_command_desc));

	cdesc->first_seg = first;
	cdesc->last_seg = last;
	cdesc->particle_size = data_len;
	cdesc->data_lo = lower_32_bits(data);
	cdesc->data_hi = upper_32_bits(data);

	if (first && context) {
		struct safexcel_token *token =
			(struct safexcel_token *)cdesc->control_data.token;

		cdesc->control_data.packet_length = full_data_len;
		cdesc->control_data.options = EIP197_OPTION_MAGIC_VALUE |
					      EIP197_OPTION_64BIT_CTX |
					      EIP197_OPTION_CTX_CTRL_IN_CMD;
		cdesc->control_data.context_lo = (lower_32_bits(context) &
						 EIP197_CONTEXT_POINTER_LO_MASK) >>
						 EIP197_CONTEXT_POINTER_LO_SHIFT;
		cdesc->control_data.context_hi = upper_32_bits(context);

		/* TODO: large xform HMAC with SHA-384/512 uses
		 * refresh = EIP197_CONTEXT_SIZE_LARGE (3)
		 */
		cdesc->control_data.refresh = EIP197_CONTEXT_SIZE_SMALL;

		for (i = 0; i < EIP197_MAX_TOKENS; i++)
			eip197_noop_token(&token[i]);
	}

	return cdesc;
}

/* Create a result descriptor */
struct safexcel_result_desc *safexcel_add_rdesc(struct safexcel_crypto_priv *priv,
						int ring_id,
						bool first, bool last,
						dma_addr_t data, u32 len)
{
	struct safexcel_result_desc *rdesc;

	rdesc = safexcel_ring_next_wptr(priv, &priv->ring[ring_id].rdr);
	if (IS_ERR(rdesc))
		return rdesc;

	memset(rdesc, 0, sizeof(struct safexcel_result_desc));

	rdesc->first_seg = first;
	rdesc->last_seg = last;
	rdesc->particle_size = len;
	rdesc->data_lo = lower_32_bits(data);
	rdesc->data_hi = upper_32_bits(data);

	return rdesc;
}
