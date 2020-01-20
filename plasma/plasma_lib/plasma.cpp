#include "plasma.h"


template<typename T>
struct Wrap : public T {
	char data[1024];
	Wrap() {
		wrapAndApplyHeader(data, 0, 1024);
	}
};

namespace plasma
{
	void OMS::OnMsg(const NewOrderSingle& req) {
		stringstream strm;
		strm << "PLS:\t\tNOS[" << ClientId(req.clOrdId()) << "]";
		std::cout << strm.str() << std::endl;
		Wrap<NewOrderSingle> nos;
		_out_os[req.target()]->OnMsg(nos);
	}
	void OMS::OnMsg(const OrderStatusRequest& req) {
		stringstream strm;
		strm << "PLS:\t\tOSR[" << ClientId(req.clOrdId()) << " / " << req.orderId() << "]";
		std::cout << strm.str() << std::endl;

		Wrap<OrderStatusRequest> osr;
		_out_os[2]->OnMsg(osr);
	}
	void OMS::OnMsg(const OrderCancelRequest& req) {
		stringstream strm;
		strm << "PLS:\t\tOCR[" << ClientId(req.clOrdId()) << "/" << ClientId(req.origClOrdId()) << "/" << req.orderId() << "]";
		std::cout << strm.str() << std::endl;
		Wrap<OrderCancelRequest> ocr;
		_out_os[2]->OnMsg(ocr);
	}
	void OMS::OnMsg(const OrderReplaceRequest& req) {
		stringstream strm;
		strm << "PLS:\t\tORR[" << ClientId(req.clOrdId()) << "/" << ClientId(req.origClOrdId()) << "/" << req.orderId() << "]";
		std::cout << strm.str() << std::endl;
		Wrap<OrderReplaceRequest> orr;
		_out_os[2]->OnMsg(orr);
	}
	void OMS::OnMsg(const NonFillReport& rpt) {
		stringstream strm;
		strm << "PLS:\t\tNTR[" << rpt.clOrdId() << "/" << rpt.origClOrdId() << "/" << rpt.orderId() << "]";
		//			strm << toString(rpt._action) << " / " << OrdStatus::c_str(rpt.status());
		strm << "[" << rpt.qty() << "," << rpt.cumQty() << "," << rpt.leavesQty() << "]";
		std::cout << strm.str() << std::endl;
		Wrap<NonFillReport> nfr;
		_out_os[2]->OnMsg(nfr);
	}
	void OMS::OnMsg(const FillReport& rpt) {
		stringstream strm;
		strm << "PLS:\t\tTRD[" << rpt.clOrdId() << "/" << rpt.orderId() << "]";
		strm << OrdStatus::c_str(rpt.status());
		strm << "[" << rpt.qty() << "," << rpt.cumQty() << "," << rpt.leavesQty() << "]";
		std::cout << strm.str() << std::endl;
		Wrap<FillReport> fill;
		_out_os[2]->OnMsg(fill);
	}
	void OMS::OnMsg(const OrderCancelReject& rpt) {
		stringstream strm;
		strm << "PLS:\t\tRJT[" << rpt.clOrdId() << "/" << rpt.origClOrdId() << "/" << rpt.orderId() << "]";
		strm << OrdStatus::c_str(rpt.status());
		std::cout << strm.str() << std::endl;
		Wrap<OrderCancelReject> rjt;
		_out_os[2]->OnMsg(rjt);
	}
	void OMS::OnMsg(const DontKnowTrade& rpt) {
		stringstream strm;
		strm << "PLS:\t\tDKT[" << rpt.orderId() << "/" << rpt.execId() << "]";
		std::cout << strm.str() << std::endl;
		Wrap<DontKnowTrade> dkt;
		_out_os[2]->OnMsg(dkt);
	}
	void OMS::OnLogin(ICallback& cb) {
		_out_os[cb.id()] = &cb;
	}
	void OMS::OnLogout(ICallback& cb) {
	}
}
