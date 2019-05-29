/** Definition of the pqxx::pipeline class.
 *
 *   Throughput-optimized query manager
 *
 * DO NOT INCLUDE THIS FILE DIRECTLY; include pqxx/pipeline instead.
 *
 * Copyright (c) 2003-2019, Jeroen T. Vermeulen.
 *
 * See COPYING for copyright license.  If you did not receive a file called
 * COPYING with this source code, please notify the distributor of this mistake,
 * or contact the author.
 */
#ifndef PQXX_H_PIPELINE
#define PQXX_H_PIPELINE

#include "pqxx/compiler-public.hxx"
#include "pqxx/compiler-internal-pre.hxx"

#include <limits>
#include <map>
#include <string>

#include "pqxx/transaction_base.hxx"


// Methods tested in eg. test module test01 are marked with "//[t01]".

namespace pqxx
{

/// Processes several queries in FIFO manner, optimized for high throughput
/** Use a pipeline if you want to execute queries without always sitting still
 * while they execute.  Result retrieval is decoupled from execution request;
 * queries "go in at the front" and results "come out the back."  Actually
 * results may be retrieved in any order, if you want.
 *
 * Feel free to pump as many queries into the pipeline as possible, even if they
 * were generated after looking at a result from the same pipeline.  To get the
 * best possible throughput, try to make insertion of queries run as far ahead
 * of results retrieval as possible; issue each query as early as possible and
 * retrieve their results as late as possible, so the pipeline has as many
 * ongoing queries as possible at any given time.  In other words, keep it busy!
 *
 * One warning: if any of the queries you insert leads to a syntactic error, the
 * error may be returned as if it were generated by an older query.  Future
 * versions may try to work around this if working in a nontransaction.
 */
class PQXX_LIBEXPORT pipeline : public internal::transactionfocus
{
public:
  using query_id = long;

  pipeline(const pipeline &) =delete;
  pipeline &operator=(const pipeline &) =delete;

  explicit pipeline(							//[t69]
	transaction_base &,
	const std::string &Name=std::string{});

  ~pipeline() noexcept;

  /// Add query to the pipeline.
  /** Queries are accumulated in the pipeline and sent to the backend in a
   * concatenated format, separated by semicolons.  The queries you insert must
   * not use this construct themselves, or the pipeline will get hopelessly
   * confused!
   * @return Identifier for this query, unique only within this pipeline
   */
  query_id insert(const std::string &);					//[t69]

  /// Wait for all ongoing or pending operations to complete.
  /** Detaches from the transaction when done. */
  void complete();							//[t71]

  /// Forget all ongoing or pending operations and retrieved results
  /** Queries already sent to the backend may still be completed, depending
   * on implementation and timing.
   *
   * Any error state (unless caused by an internal error) will also be cleared.
   * This is mostly useful in a nontransaction, since a backend transaction is
   * aborted automatically when an error occurs.
   *
   * Detaches from the transaction when done.
   */
  void flush();								//[t70]

  /// Cancel ongoing query, if any.
  /** May cancel any or all of the queries that have been inserted at this point
   * whose results have not yet been retrieved.  If the pipeline lives in a
   * backend transaction, that transaction may be left in a nonfunctional state
   * in which it can only be aborted.
   *
   * Therefore, either use this function in a nontransaction, or abort the
   * transaction after calling it.
   */
  void cancel();

  /// Is result for given query available?
  bool is_finished(query_id) const;					//[t71]

  /// Retrieve result for given query
  /** If the query failed for whatever reason, this will throw an exception.
   * The function will block if the query has not finished yet.
   * @warning If results are retrieved out-of-order, i.e. in a different order
   * than the one in which their queries were inserted, errors may "propagate"
   * to subsequent queries.
   */
  result retrieve(query_id qid)						//[t71]
	{ return retrieve(m_queries.find(qid)).second; }

  /// Retrieve oldest unretrieved result (possibly wait for one)
  /** @return The query's identifier and its result set */
  std::pair<query_id, result> retrieve();				//[t69]

  bool empty() const noexcept { return m_queries.empty(); }		//[t69]

  /// Set maximum number of queries to retain before issuing them to the backend
  /** The pipeline will perform better if multiple queries are issued at once,
   * but retaining queries until the results are needed (as opposed to issuing
   * them to the backend immediately) may negate any performance benefits the
   * pipeline can offer.
   *
   * Recommended practice is to set this value no higher than the number of
   * queries you intend to insert at a time.
   * @param retain_max A nonnegative "retention capacity;" passing zero will
   * cause queries to be issued immediately
   * @return Old retention capacity
   */
  int retain(int retain_max=2);						//[t70]


  /// Resume retained query emission (harmless when not needed)
  void resume();							//[t70]

private:
  class PQXX_PRIVATE Query
  {
  public:
    explicit Query(const std::string &q) : m_query{q}, m_res{} {}

    const result &get_result() const noexcept { return m_res; }
    void set_result(const result &r) noexcept { m_res = r; }
    const std::string &get_query() const noexcept { return m_query; }

  private:
    std::string m_query;
    result m_res;
  };

  using QueryMap = std::map<query_id,Query>;

  struct getquery
  {
    getquery(){}	// Silences bogus warning in some gcc versions
    std::string operator()(QueryMap::const_iterator i) const
	{ return i->second.get_query(); }
  };

  void attach();
  void detach();

  /// Upper bound to query id's
  static constexpr query_id qid_limit() noexcept
  {
    // Parenthesise this to work around an eternal Visual C++ problem:
    // Without the extra parentheses, unless NOMINMAX is defined, the
    // preprocessor will mistake this "max" for its annoying built-in macro
    // of the same name.
    return (std::numeric_limits<query_id>::max)();
  }

  /// Create new query_id
  PQXX_PRIVATE query_id generate_id();

  bool have_pending() const noexcept
	{ return m_issuedrange.second != m_issuedrange.first; }

  PQXX_PRIVATE void issue();

  /// The given query failed; never issue anything beyond that
  void set_error_at(query_id qid) noexcept
	{ if (qid < m_error) m_error = qid; }

  /// Throw pqxx::internal_error.
  [[noreturn]] PQXX_PRIVATE void internal_error(const std::string &err);

  PQXX_PRIVATE bool obtain_result(bool expect_none=false);

  PQXX_PRIVATE void obtain_dummy();
  PQXX_PRIVATE void get_further_available_results();
  PQXX_PRIVATE void check_end_results();

  /// Receive any results that happen to be available; it's not urgent
  PQXX_PRIVATE void receive_if_available();

  /// Receive results, up to stop if possible
  PQXX_PRIVATE void receive(pipeline::QueryMap::const_iterator stop);
  std::pair<pipeline::query_id, result>
    retrieve(pipeline::QueryMap::iterator);

  QueryMap m_queries;
  std::pair<QueryMap::iterator,QueryMap::iterator> m_issuedrange;
  int m_retain = 0;
  int m_num_waiting = 0;
  query_id m_q_id = 0;

  /// Is there a "dummy query" pending?
  bool m_dummy_pending = false;

  /// Point at which an error occurred; no results beyond it will be available
  query_id m_error = qid_limit();
};

} // namespace

#include "pqxx/compiler-internal-post.hxx"
#endif
