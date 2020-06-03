export * from "@solo-io/proxy-runtime/proxy"; // this exports the required functions for the proxy to interact with us.
import {
  RootContext,
  Context,
  RootContextHelper,
  ContextHelper,
  registerRootContext,
  FilterHeadersStatusValues,
  stream_context,
  log,
  LogLevelValues,
} from "@solo-io/proxy-runtime";

class AddHeaderRoot extends RootContext {
  configuration: string;

  createContext(context_id: u32): Context {
    return ContextHelper.wrap(new AddHeader(context_id, this));
  }
}

class AddHeader extends Context {
  root_context: AddHeaderRoot;
  constructor(context_id: u32, root_context: AddHeaderRoot) {
    super(context_id, root_context);
    this.root_context = root_context;
  }

  onRequestHeaders(a: u32): FilterHeadersStatusValues {
    stream_context.headers.request.add("x-b3-sampled", "1");
    let headers = stream_context.headers.request.get_headers();
    for (let i = 0; i < headers.length; ++i) {
      log(
        LogLevelValues.debug,
        headers[i].key.toString() + " -> " + headers[i].value.toString()
      );
    }

    return FilterHeadersStatusValues.Continue;
  }
  onResponseHeaders(a: u32): FilterHeadersStatusValues {
    let headers = stream_context.headers.response.get_headers();
    for (let i = 0; i < headers.length; ++i) {
      log(
        LogLevelValues.debug,
        headers[i].key.toString() + " -> " + headers[i].value.toString()
      );
    }
    return FilterHeadersStatusValues.Continue;
  }
}

registerRootContext((context_id: u32) => {
  return RootContextHelper.wrap(new AddHeaderRoot(context_id));
}, "add_header");
