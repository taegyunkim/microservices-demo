export * from "@solo-io/proxy-runtime/proxy"; // this exports the required functions for the proxy to interact with us.
import {
  RootContext,
  Context,
  RootContextHelper,
  ContextHelper,
  registerRootContext,
  FilterHeadersStatusValues,
  stream_context,
  LogLevelValues,
  log,
} from "@solo-io/proxy-runtime";

class AddHeaderRoot extends RootContext {
  next_trace_id_: u32;
  configuration: string;

  constructor(context_id: u32) {
    super(context_id);
    this.next_trace_id_ = 0;
  }

  createContext(context_id: u32): Context {
    return ContextHelper.wrap(new AddHeader(context_id, this));
  }

  getNextTraceId(): u32 {
    let ret = this.next_trace_id_;
    this.next_trace_id_ += 1;
    return ret;
  }
}

class AddHeader extends Context {
  root_context: AddHeaderRoot;

  constructor(context_id: u32, root_context: AddHeaderRoot) {
    super(context_id, root_context);
    this.root_context = root_context;
  }

  onRequestHeaders(a: u32): FilterHeadersStatusValues {
    if (stream_context.headers.request.get("x-wasm-trace-id") == "") {
      stream_context.headers.request.add(
        "x-wasm-trace-id",
        this.root_context.getNextTraceId().toString()
      );
    }

    let pairs = stream_context.headers.request.get_headers();
    for (let i = 0; i < pairs.length; ++i) {
      log(
        LogLevelValues.debug,
        pairs[i].key.toString() + " -> " + pairs[i].value.toString()
      );
    }
    return FilterHeadersStatusValues.Continue;
  }

  onResponseHeaders(a: u32): FilterHeadersStatusValues {
    stream_context.headers.response.add("hello", "world!");

    let pairs = stream_context.headers.response.get_headers();
    for (let i = 0; i < pairs.length; ++i) {
      log(
        LogLevelValues.debug,
        pairs[i].key.toString() + " -> " + pairs[i].value.toString()
      );
    }
    return FilterHeadersStatusValues.Continue;
  }
}

registerRootContext((context_id: u32) => {
  return RootContextHelper.wrap(new AddHeaderRoot(context_id));
}, "add_header");
