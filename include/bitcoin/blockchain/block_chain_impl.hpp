/**
 * Copyright (c) 2011-2015 libbitcoin developers (see AUTHORS)
 *
 * This file is part of libbitcoin.
 *
 * libbitcoin is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License with
 * additional permissions to the one published by the Free Software
 * Foundation, either version 3 of the License, or (at your option)
 * any later version. For more information see LICENSE.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef LIBBITCOIN_BLOCKCHAIN_BLOCK_CHAIN_IMPL_HPP
#define LIBBITCOIN_BLOCKCHAIN_BLOCK_CHAIN_IMPL_HPP

#include <atomic>
#include <cstddef>
#include <cstdint>
#include <vector>
#include <bitcoin/bitcoin.hpp>
#include <bitcoin/database.hpp>
#include <bitcoin/blockchain/block_chain.hpp>
#include <bitcoin/blockchain/define.hpp>
#include <bitcoin/blockchain/organizer.hpp>
#include <bitcoin/blockchain/settings.hpp>
#include <bitcoin/blockchain/simple_chain.hpp>
#include <bitcoin/blockchain/transaction_pool.hpp>

namespace libbitcoin {
namespace blockchain {

/// The simple_chain interface portion of this class is not thread safe.
class BCB_API block_chain_impl
  : public block_chain, public simple_chain
{
public:
    block_chain_impl(threadpool& pool, 
        const blockchain::settings& chain_settings,
        const database::settings& database_settings);

    /// The database is closed on destruct, threads must be joined.
    ~block_chain_impl();

    /// This class is not copyable.
    block_chain_impl(const block_chain_impl&) = delete;
    void operator=(const block_chain_impl&) = delete;

    // Properties (thread safe).
    // ------------------------------------------------------------------------

    // Get a reference to the transaction pool.
    transaction_pool& pool();

    // Get a reference to the blockchain configuration settings.
    const settings& chain_settings() const;

    // block_chain start/stop (thread safe).
    // ------------------------------------------------------------------------

    /// Start or restart the blockchain.
    virtual bool start();

    /// Signal stop of current work, speeds shutdown with multiple threads.
    virtual bool stop();

    /// Close the blockchain, threads must first be joined, can be restarted.
    virtual bool close();

    // simple_chain (NOT THREAD SAFE).
    // ------------------------------------------------------------------------

    /// Return the first and last gaps in the blockchain, or false if none.
    bool get_gap_range(uint64_t& out_first, uint64_t& out_last) const;

    /// Return the next chain gap at or after the specified start height.
    bool get_next_gap(uint64_t& out_height, uint64_t start_height) const;

    /// Get the dificulty of a block at the given height.
    bool get_difficulty(hash_number& out_difficulty, uint64_t height) const;

    /// Get the header of the block at the given height.
    bool get_header(chain::header& out_header, uint64_t height) const;

    /// Get the height of the block with the given hash.
    bool get_height(uint64_t& out_height, const hash_digest& block_hash) const;

    /// Get height of latest block.
    bool get_last_height(uint64_t& out_height) const;

    /// Get the hash digest of the transaction of the outpoint.
    bool get_outpoint_transaction(hash_digest& out_hash,
        const chain::output_point& outpoint) const;

    /// Get the transaction of the given hash and its block height.
    transaction_ptr get_transaction(uint64_t& out_block_height,
        const hash_digest& transaction_hash) const;

    /// Get the block height of the transaction given its hash.
    bool get_transaction_height(uint64_t& out_block_height,
        const hash_digest& transaction_hash) const;

    /// Import a block to the blockchain.
    bool import(block_const_ptr block, uint64_t height);

    /// Append the block to the top of the chain.
    bool push(block_const_ptr block);

    /// Remove blocks at or above the given height, returning them in order.
    bool pop_from(block_const_ptr_list& out_blocks, uint64_t height);

    // block_chain queries (thread safe).
    // ------------------------------------------------------------------------

    /// Store a block to the blockchain, with indexing and validation.
    virtual void store(block_const_ptr block, block_store_handler handler);
    
    /// fetch a block by height.
    virtual void fetch_block(uint64_t height,
        block_fetch_handler handler) const;

    /// fetch a block by height.
    virtual void fetch_block(const hash_digest& hash,
        block_fetch_handler handler) const;

    /// fetch block header by height.
    virtual void fetch_block_header(uint64_t height,
        block_header_fetch_handler handler) const;

    /// fetch block header by hash.
    virtual void fetch_block_header(const hash_digest& hash,
        block_header_fetch_handler handler) const;

    /// fetch hashes of transactions for a block, by block height.
    virtual void fetch_block_transaction_hashes(uint64_t height,
        transaction_hashes_fetch_handler handler) const;

    /// fetch hashes of transactions for a block, by block hash.
    virtual void fetch_block_transaction_hashes(const hash_digest& hash,
        transaction_hashes_fetch_handler handler) const;

    /// fetch a block locator relative to the current top and threshold.
    virtual void fetch_block_locator(
        block_locator_fetch_handler handler) const;

    /// fetch the set of block hashes indicated by the block locator.
    virtual void fetch_locator_block_hashes(get_blocks_const_ptr locator,
        const hash_digest& threshold, size_t limit,
        locator_block_hashes_fetch_handler handler) const;

    /// fetch the set of block headers indicated by the block locator.
    virtual void fetch_locator_block_headers(get_headers_const_ptr locator,
        const hash_digest& threshold, size_t limit,
        locator_block_headers_fetch_handler handler) const;

    /// fetch height of block by hash.
    virtual void fetch_block_height(const hash_digest& hash,
        block_height_fetch_handler handler) const;

    /// fetch height of latest block.
    virtual void fetch_last_height(last_height_fetch_handler handler) const;

    /// fetch transaction by hash.
    virtual void fetch_transaction(const hash_digest& hash,
        transaction_fetch_handler handler) const;

    /// fetch height and offset within block of transaction by hash.
    virtual void fetch_transaction_index(const hash_digest& hash,
        transaction_index_fetch_handler handler) const;

    /// fetch spend of an output point.
    virtual void fetch_spend(const chain::output_point& outpoint,
        spend_fetch_handler handler) const;

    /// fetch outputs, values and spends for an address.
    virtual void fetch_history(const wallet::payment_address& address,
        uint64_t limit, uint64_t from_height,
        history_fetch_handler handler) const;

    /// fetch stealth results.
    virtual void fetch_stealth(const binary& filter, uint64_t from_height,
        stealth_fetch_handler handler) const;

    /// filter out block hashes that exist in the store.
    virtual void filter_blocks(get_data_ptr message,
        result_handler handler) const;

    /// filter out block hashes that exist in the orphan pool.
    virtual void filter_orphans(get_data_ptr message,
        result_handler handler) const;

    /// filter out transaction hashes that exist in the store.
    virtual void filter_transactions(get_data_ptr message,
        result_handler handler) const;

    /// Subscribe to blockchain reorganizations.
    virtual void subscribe_reorganize(reorganize_handler handler);

private:
    typedef std::function<bool(database::handle)> perform_read_functor;

    template <typename Handler, typename... Args>
    bool finish_fetch(database::handle handle,
        Handler handler, Args&&... args) const
    {
        if (!database_.is_read_valid(handle))
            return false;

        handler(std::forward<Args>(args)...);
        return true;
    }

    template <typename Handler, typename... Args>
    void stop_write(Handler handler, Args&&... args)
    {
        const auto result = database_.end_write();
        BITCOIN_ASSERT(result);
        handler(std::forward<Args>(args)...);
    }

    void start_write();
    void do_store(block_const_ptr block, block_store_handler handler);

    void fetch_serial(perform_read_functor perform_read) const;
    bool stopped() const;

    std::atomic<bool> stopped_;
    const settings& settings_;

    // These are thread safe.
    organizer organizer_;
    ////mutable dispatcher read_dispatch_;
    ////mutable dispatcher write_dispatch_;
    blockchain::transaction_pool transaction_pool_;

    // This is protected by mutex.
    database::data_base database_;
    mutable shared_mutex mutex_;
};

} // namespace blockchain
} // namespace libbitcoin

#endif
