// plasma.cpp : Defines the entry point for the application.
//

#include "plasma.h"
#include <plasma_client/NewOrderSingle.h>
#include <Aeron.h>

using namespace std;

int main()
{
	aeron::Context ctxt;
	ctxt.newSubscriptionHandler(
		[](const std::string& channel, std::int32_t streamId, std::int64_t correlationId) {
			std::cout << "Subscription: " << channel << " " << correlationId << ":" << streamId << std::endl;
		});

	aeron::Aeron aeron(ctxt);

	auto id = aeron.addSubscription("aeron:ipc", 55, [](aeron::Image& image)
		{
			std::cout << "Available image correlationId=" << image.correlationId() << " sessionId=" << image.sessionId();
			std::cout << " at position=" << image.position() << " from " << image.sourceIdentity() << std::endl;
		},
		[](aeron::Image& image)
		{
			std::cout << "Unavailable image on correlationId=" << image.correlationId() << " sessionId=" << image.sessionId();
			std::cout << " at position=" << image.position() << " from " << image.sourceIdentity() << std::endl;
		});

	std::shared_ptr<aeron::Subscription> subscription = aeron.findSubscription(id);
	// wait for the subscription to be valid
	while (!subscription) {
		std::this_thread::yield();
		subscription = aeron.findSubscription(id);
	}
	// check status
	const std::int64_t channelStatus = subscription->channelStatus();
	std::cout << "Subscription channel status (id=" << subscription->channelStatusId() << ") "
		<< (channelStatus == aeron::ChannelEndpointStatus::CHANNEL_ENDPOINT_ACTIVE ? "ACTIVE" : std::to_string(channelStatus))
		<< std::endl;

	while (true) {
	}
	cout << "Hello CMake." << plasma::client::NewOrderSingle::sbeSchemaVersion() << endl;
	return 0;
}
