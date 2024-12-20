// Copyright 2020-2023 Bloomberg Finance L.P.
// SPDX-License-Identifier: Apache-2.0
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

// rmqa_rabbitcontextoptions.h
#ifndef INCLUDED_RMQA_RABBITCONTEXTOPTIONS
#define INCLUDED_RMQA_RABBITCONTEXTOPTIONS

#include <rmqp_metricpublisher.h>

#include <rmqp_consumertracing.h>
#include <rmqp_producertracing.h>
#include <rmqt_fieldvalue.h>
#include <rmqt_properties.h>
#include <rmqt_result.h>

#include <bdlmt_threadpool.h>
#include <bsl_memory.h>
#include <bsl_set.h>
#include <bsls_timeinterval.h>

#include <rmqa_connectionmonitor.h>

namespace BloombergLP {
namespace rmqa {

/// \brief Class for passing arguments to RabbitContext
///
/// This class provides passing arguments to RabbitContext. It allows specifying
/// the async callback threadpool, metric publisher and error callback. If any
/// of them are not specified, a default value is used.

class RabbitContextOptions {
  public:
    typedef bsl::set<bsl::string> Tunables;

    /// \brief By Default RabbitContext will
    /// 1) Create it's own threadpool for
    /// calling back to client code e.g. consuming messages, confirming
    /// published messages
    /// 2) Stub out error notifications - this library retries indefinitely for
    /// operations requested of it, set an error handler to be made aware of
    /// ongoing retry attempts in order to circuit break the retries for your
    /// applications needs
    /// 3) Stub out metric publishing, if you would like to publish metrics
    /// please provide an implementation that confirms to the MetricPublisher
    /// interface
    RabbitContextOptions();

    /// \param threadpool For async callbacks (e.g. as messages arrived for
    /// Consumer, and as confirmations arrive for Producer). By
    /// default a threadpool is created with the following parameters:
    ///     rmqp::Connection::s_threadPoolMinThreads
    ///     rmqp::Connection::s_threadPoolMaxThreads
    ///     rmqp::Connection::s_threadPoolMaxIdleTimeMs
    /// A custom threadpool can be provided if it is desirable for async
    /// callbacks to be executed in a shared threadpool, or to tune this
    /// threadpool. The provided threadpool must live longer than the
    /// RabbitContext
    RabbitContextOptions& setThreadpool(bdlmt::ThreadPool* threadpool);

    /// \param metricPublisher custom metric publisher will be used
    /// to publish different types of metrics generated by the library.
    RabbitContextOptions& setMetricPublisher(
        const bsl::shared_ptr<rmqp::MetricPublisher>& metricPublisher);

    /// \param errorCallback function will be called with error detail,
    /// when channel or connection is closed by rabbitmq broker.
    RabbitContextOptions&
    setErrorCallback(const rmqt::ErrorCallback& errorCallback);

    /// \param successCallback function will be called when channel or
    /// connection is restored
    RabbitContextOptions& setSuccessCallback(const rmqt::SuccessCallback& successCallback);

    /// \param hungMessageCallback function will be called when a connection monitor
    /// detects hung messages
    RabbitContextOptions& setHungMessageCallback(const rmqa::ConnectionMonitor::HungMessageCallback& callback);

    /// \param name name of client property to set
    /// \param value value of client property
    /// NOTE: The following properties are set by default and can be
    /// overridden: task, pid, os, os_version, os_patch. The following
    /// properties are reserved and cannot be overridden: capabilities,
    /// platform, product, version, connection_name
    RabbitContextOptions& setClientProperty(const bsl::string& name,
                                            const rmqt::FieldValue& value);

    /// \brief Set time in which consumers should process messages
    /// \param timeout Message processing timeout
    /// This timeout specifies the time interval in which a consumer is
    /// supposed to process a message. If a consumer takes longer that the
    /// specified timeout, warnings will be logged.
    /// \note the library detects such cases periodically and the warnings
    /// generally won't be logged immediately after the given timeout expires.
    /// False-positive warnings may still be logged if the library has not
    /// processed the consumer acknowledgement before the timeout expires.
    RabbitContextOptions&
    setMessageProcessingTimeout(const bsls::TimeInterval& timeout);

    /// \brief Set time threshold at which point the error callback is called
    /// if there has been no success in establishing an amqp connection to the
    /// broker
    /// \param timeout the timeout value
    RabbitContextOptions& setConnectionErrorThreshold(
        const bsl::optional<bsls::TimeInterval>& timeout);

    /// \brief will be called back to create a context which spans for the
    /// lifetime of the messageguard _before_ it is passed to its consumer
    /// message processor if there has
    /// \param consumerTracing implements the
    /// rmqp::ConsumerTracing protocol
    RabbitContextOptions& setConsumerTracing(
        const bsl::shared_ptr<rmqp::ConsumerTracing>& consumerTracing);

    /// \brief will be called back at message send, with meta data to establish
    /// a context, the context will be kept alive until the messageConfirm
    /// response callback
    /// \param producerTracing implements the
    /// rmqp::ProducerTracing protocol
    RabbitContextOptions& setProducerTracing(
        const bsl::shared_ptr<rmqp::ProducerTracing>& producerTracing);

    /// \brief DEPRECATED: Previously was used to switch between AMQP-spec
    /// and RabbitMQ-spec Field Value encoding. This is now always true
    RabbitContextOptions& useRabbitMQFieldValueEncoding(bool rabbitEncoding);

    /// \brief Shuffle endpoints rmq connects to.
    /// By default, boost asio (libc) resolves and connects
    /// to the node with longest matching subnet prefix
    /// causing disproportionately more connections with certain endpoints.
    /// Setting this option will shuffle resolver results.
    /// \param shuffleConnectionEndpoints set to true to shuffle.
    RabbitContextOptions&
    setShuffleConnectionEndpoints(bool shuffleConnectionEndpoints);

    bdlmt::ThreadPool* threadpool() const { return d_threadpool; }

    const bsl::shared_ptr<rmqp::MetricPublisher>& metricPublisher() const
    {
        return d_metricPublisher;
    }

    const rmqt::ErrorCallback& errorCallback() const { return d_onError; }

    const rmqt::SuccessCallback& successCallback() const { return d_onSuccess; }

    rmqa::ConnectionMonitor::HungMessageCallback d_onHungMessage;

    const rmqt::FieldTable& clientProperties() const
    {
        return d_clientProperties;
    }

    const bsls::TimeInterval& messageProcessingTimeout() const
    {
        return d_messageProcessingTimeout;
    }

    const bsl::optional<bsls::TimeInterval>& connectionErrorThreshold() const
    {
        return d_connectionErrorThreshold;
    }

    const rmqt::Tunables& tunables() const { return d_tunables; }

    const bsl::shared_ptr<rmqp::ConsumerTracing>& consumerTracing() const
    {
        return d_consumerTracing;
    }

    const bsl::shared_ptr<rmqp::ProducerTracing>& producerTracing() const
    {
        return d_producerTracing;
    }

    const bsl::optional<bool>& shuffleConnectionEndpoints() const
    {
        return d_shuffleConnectionEndpoints;
    }

#ifdef USES_LIBRMQ_EXPERIMENTAL_FEATURES
    RabbitContextOptions& setTunable(const bsl::string& tunable);
#endif

  private:
    static const int DEFAULT_MESSAGE_PROCESSING_TIMEOUT = 60;
    bdlmt::ThreadPool* d_threadpool;
    rmqt::ErrorCallback d_onError;
    rmqt::SuccessCallback d_onSuccess;
    bsl::shared_ptr<rmqp::MetricPublisher> d_metricPublisher;
    rmqt::FieldTable d_clientProperties;
    bsls::TimeInterval d_messageProcessingTimeout;
    rmqt::Tunables d_tunables;
    bsl::optional<bsls::TimeInterval> d_connectionErrorThreshold;
    bsl::shared_ptr<rmqp::ConsumerTracing> d_consumerTracing;
    bsl::shared_ptr<rmqp::ProducerTracing> d_producerTracing;
    bsl::optional<bool> d_shuffleConnectionEndpoints;
};

} // namespace rmqa
} // namespace BloombergLP

#endif
