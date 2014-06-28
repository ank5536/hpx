//  Copyright (c) 2014 Grant Mercer
//
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !defined(HPX_PARALLEL_DETAIL_COPY_MAY_30_2014_0317PM)
#define HPX_PARALLEL_DETAIL_COPY_MAY_30_2014_0317PM

#include <hpx/hpx_fwd.hpp>
#include <hpx/exception_list.hpp>
#include <hpx/parallel/execution_policy.hpp>
#include <hpx/parallel/detail/algorithm_result.hpp>
#include <hpx/parallel/detail/zip_iterator.hpp>

#include <algorithm>
#include <iterator>

#include <boost/static_assert.hpp>
#include <boost/utility/enable_if.hpp>
#include <boost/type_traits/is_base_of.hpp>

namespace hpx { namespace parallel
{
    ///////////////////////////////////////////////////////////////////////////
    // copy
    namespace detail
    {
        template <typename ExPolicy, typename InIter, typename OutIter>
        typename detail::algorithm_result<ExPolicy, OutIter>::type
        copy(ExPolicy const&, InIter first, InIter last, OutIter dest,
            boost::mpl::true_)
        {
            try {
                return detail::algorithm_result<ExPolicy, OutIter>::get(
                    std::copy(first, last, dest));
            }
            catch (...) {
                detail::handle_exception<ExPolicy>::call();
            }
        }

        template <typename ExPolicy, typename FwdIter, typename OutIter>
        typename detail::algorithm_result<ExPolicy, OutIter>::type
        copy(ExPolicy const& policy, FwdIter first, FwdIter last, OutIter dest,
            boost::mpl::false_ fls)
        {
            typedef boost::tuple<FwdIter, OutIter> iterator_tuple;
            typedef detail::zip_iterator<iterator_tuple> zip_iterator;
            typedef typename zip_iterator::reference reference;
            typedef
                typename detail::algorithm_result<ExPolicy, OutIter>::type
            result_type;

            return get_iter<1, result_type>(
                plain_for_each_n(policy,
                    detail::make_zip_iterator(boost::make_tuple(first, dest)),
                    std::distance(first, last),
                    [](reference it) {
                        *boost::get<1>(it) = *boost::get<0>(it);
                    },
                    fls));
        }

        template <typename InIter, typename OutIter>
        OutIter copy(execution_policy const& policy,
            InIter first, InIter last, OutIter dest, boost::mpl::false_)
        {
            HPX_PARALLEL_DISPATCH(policy, detail::copy, first, last, dest);
        }

        template <typename InIter, typename OutIter>
        OutIter copy(execution_policy const& policy,
            InIter first, InIter last, OutIter dest, boost::mpl::true_ t)
        {
            return detail::copy(sequential_execution_policy(),
                first, last, dest, t);
        }
    }

    template <typename ExPolicy, typename InIter, typename OutIter>
    inline typename boost::enable_if<
        is_execution_policy<ExPolicy>,
        typename detail::algorithm_result<ExPolicy, OutIter>::type
    >::type
    copy(ExPolicy && policy, InIter first, InIter last, OutIter dest)
    {
        typedef typename std::iterator_traits<InIter>::iterator_category
            input_iterator_category;
        typedef typename std::iterator_traits<OutIter>::iterator_category
            output_iterator_category;

        BOOST_STATIC_ASSERT_MSG(
            boost::is_base_of<
                std::input_iterator_tag, input_iterator_category>::value,
            "Required at least input iterator.");

        BOOST_STATIC_ASSERT_MSG(
            boost::mpl::or_<
                boost::is_base_of<
                    std::forward_iterator_tag, output_iterator_category>,
                boost::is_same<
                    std::output_iterator_tag, output_iterator_category>
            >::value,
            "Requires at least output iterator.");

        typedef typename boost::mpl::or_<
            is_sequential_execution_policy<ExPolicy>,
            boost::is_same<std::input_iterator_tag, input_iterator_category>,
            boost::is_same<std::output_iterator_tag, output_iterator_category>
        >::type is_seq;

        return detail::copy(std::forward<ExPolicy>(policy),
            first, last, dest, is_seq());
    }

    /////////////////////////////////////////////////////////////////////////////
    // copy_n
    namespace detail
    {
        template <typename ExPolicy, typename InIter, typename OutIter>
        typename detail::algorithm_result<ExPolicy, OutIter>::type
        copy_n(ExPolicy const&, InIter first, std::size_t count, OutIter dest,
            boost::mpl::true_)
        {
            typedef boost::tuple<InIter, OutIter> iterator_tuple;
            typedef detail::zip_iterator<iterator_tuple> zip_iterator;
            typedef typename zip_iterator::reference reference;

            try {
                return detail::algorithm_result<ExPolicy, OutIter>::get(
                    get_iter<1, OutIter>(
                        plain_for_each_n(sequential_execution_policy(),
                            detail::make_zip_iterator(boost::make_tuple(first, dest)),
                            count,
                            [](reference it) {
                                *boost::get<1>(it) = *boost::get<0>(it);
                            },
                            boost::mpl::true_())
                    ));
            }
            catch(...) {
                detail::handle_exception<ExPolicy>::call();
            }
        }

        template <typename ExPolicy, typename FwdIter, typename OutIter>
        typename detail::algorithm_result<ExPolicy, OutIter>::type
        copy_n(ExPolicy const& policy, FwdIter first, std::size_t count,
            OutIter dest, boost::mpl::false_ fls)
        {
            typedef boost::tuple<FwdIter,OutIter> iterator_tuple;
            typedef detail::zip_iterator<iterator_tuple> zip_iterator;
            typedef typename zip_iterator::reference reference;
            typedef
                typename detail::algorithm_result<ExPolicy, OutIter>::type
            result_type;

            return get_iter<1, result_type>(
                plain_for_each_n(policy,
                    detail::make_zip_iterator(boost::make_tuple(first, dest)),
                    count,
                    [](reference it) {
                        *boost::get<1>(it) = *boost::get<0>(it);
                    },
                    fls));
        }

        template <typename InIter, typename OutIter>
        OutIter copy_n(execution_policy const& policy,
            InIter first, std::size_t count, OutIter dest, boost::mpl::false_)
        {
            HPX_PARALLEL_DISPATCH(policy, detail::copy_n, first, count, dest);
        }

        template <typename InIter, typename OutIter>
        OutIter copy_n(execution_policy const& policy,
            InIter first, std::size_t count, OutIter dest, boost::mpl::true_ t)
        {
            return detail::copy_n(sequential_execution_policy(),
                first, count, dest, t);
        }
    }

    template <typename ExPolicy, typename InIter, typename OutIter>
    typename boost::enable_if<
        is_execution_policy<ExPolicy>,
        typename detail::algorithm_result<ExPolicy, OutIter>::type
    >::type
    copy_n(ExPolicy && policy, InIter first, std::size_t count, OutIter dest)
    {
        typedef typename std::iterator_traits<InIter>::iterator_category
            input_iterator_category;
        typedef typename std::iterator_traits<OutIter>::iterator_category
            output_iterator_category;

        BOOST_STATIC_ASSERT_MSG(
            boost::is_base_of<
                std::input_iterator_tag, input_iterator_category>::value,
            "Required at least input iterator.");

        BOOST_STATIC_ASSERT_MSG(
            boost::mpl::or_<
                boost::is_base_of<
                    std::forward_iterator_tag, output_iterator_category>,
                boost::is_same<
                    std::output_iterator_tag, output_iterator_category>
            >::value,
            "Requires at least output iterator.");

        typedef typename boost::mpl::or_<
            is_sequential_execution_policy<ExPolicy>,
            boost::is_same<std::input_iterator_tag, input_iterator_category>,
            boost::is_same<std::output_iterator_tag, output_iterator_category>
        >::type is_seq;

        return detail::copy_n(std::forward<ExPolicy>(policy),
            first, count, dest, is_seq());
    }

    /////////////////////////////////////////////////////////////////////////////
    // copy_if
    namespace detail
    {
        template <typename ExPolicy, typename InIter, typename OutIter,
            typename F>
        typename detail::algorithm_result<ExPolicy, OutIter>::type
        copy_if(ExPolicy const&, InIter first, InIter last, OutIter dest,
            F && f, boost::mpl::true_)
        {
            typedef boost::tuple<InIter, OutIter> iterator_tuple;
            typedef detail::zip_iterator<iterator_tuple> zip_iterator;
            typedef typename zip_iterator::reference reference;

            try{
                return detail::algorithm_result<ExPolicy, OutIter>::get(
                    get_iter<1, OutIter>(
                        plain_for_each_n(sequential_execution_policy(),
                            detail::make_zip_iterator(boost::make_tuple(first, dest)),
                            std::distance(first, last),
                            [f](reference it) {
                                if (f(*boost::get<0>(it)))
                                    *boost::get<1>(it) = *boost::get<0>(it);
                            },
                            boost::mpl::true_())
                    )
                );
            }
            catch(...) {
                detail::handle_exception<ExPolicy>::call();
            }
        }

        template <typename ExPolicy, typename FwdIter, typename OutIter,
            typename F>
        typename detail::algorithm_result<ExPolicy, OutIter>::type
        copy_if(ExPolicy const& policy, FwdIter first, FwdIter last, OutIter dest,
            F && f, boost::mpl::false_ fls)
        {
            typedef boost::tuple<FwdIter, OutIter> iterator_tuple;
            typedef detail::zip_iterator<iterator_tuple> zip_iterator;
            typedef typename zip_iterator::reference reference;
            typedef
                typename detail::algorithm_result<ExPolicy, OutIter>::type
            result_type;

            return get_iter<1, result_type>(
                plain_for_each_n(policy,
                    detail::make_zip_iterator(boost::make_tuple(first, dest)),
                    std::distance(first,last),
                    [f](reference it) {
                        if (f(*boost::get<0>(it)))
                            *boost::get<1>(it) = *boost::get<0>(it);
                    },
                    fls));
        }

        template <typename InIter, typename OutIter, typename F>
        OutIter copy_if(execution_policy const& policy,
            InIter first, InIter last, OutIter dest, F && f, boost::mpl::false_)
        {
            HPX_PARALLEL_DISPATCH(policy, detail::copy_if, first, last, dest,
                std::forward<F>(f));
        }

        template <typename InIter, typename OutIter, typename F>
        OutIter copy_if(execution_policy const& policy,
            InIter first, InIter last, OutIter dest, F && f, boost::mpl::true_ t)
        {
            return detail::copy_if(sequential_execution_policy(),
                first, last, dest, std::forward<F>(f), t);
        }
    }

    template <typename ExPolicy, typename InIter, typename OutIter, typename F>
    inline typename boost::enable_if<
        is_execution_policy<ExPolicy>,
        typename detail::algorithm_result<ExPolicy, OutIter>::type
    >::type
    copy_if(ExPolicy&& policy, InIter first, InIter last, OutIter dest, F && f)
    {
        typedef typename std::iterator_traits<InIter>::iterator_category
            input_iterator_category;
        typedef typename std::iterator_traits<OutIter>::iterator_category
            output_iterator_category;

        BOOST_STATIC_ASSERT_MSG(
            boost::is_base_of<
                std::input_iterator_tag, input_iterator_category>::value,
            "Required at least input iterator.");

        BOOST_STATIC_ASSERT_MSG(
            boost::mpl::or_<
                boost::is_base_of<
                    std::forward_iterator_tag, output_iterator_category>,
                boost::is_same<
                    std::output_iterator_tag, output_iterator_category>
            >::value,
            "Requires at least output iterator.");

        typedef typename boost::mpl::or_<
            is_sequential_execution_policy<ExPolicy>,
            boost::is_same<std::input_iterator_tag, input_iterator_category>,
            boost::is_same<std::output_iterator_tag, output_iterator_category>
        >::type is_seq;

        return detail::copy_if(std::forward<ExPolicy>(policy),
            first, last, dest,
            std::forward<F>(f), is_seq());
    }
}}

#endif