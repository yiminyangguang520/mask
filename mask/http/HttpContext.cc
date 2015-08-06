#include <mask/http/HttpContext.h>
#include <muduo/base/Logging.h>
#include <muduo/net/Buffer.h>

using namespace mask;
using namespace mask::http;

HttpContext::HttpContext()
  : receiveTime_(0),
    lastHeaderCallback_(kHeaderNullCallback),
    requestComplete_(false)

{
  http_parser_init(&parser_, HTTP_REQUEST);
  parser_.data = this;

  http_parser_settings_init(&settings_);
  settings_.on_message_begin = HttpContext::onMessageBegin;
  settings_.on_url = HttpContext::onUrl;
  settings_.on_status = HttpContext::onStatus;
  settings_.on_header_field = HttpContext::onHeaderField;
  settings_.on_header_value = HttpContext::onHeaderValue;
  settings_.on_headers_complete = HttpContext::onHeadersComplete;
  settings_.on_body = HttpContext::onBody;
  settings_.on_message_complete = HttpContext::onMessageComplete;
  settings_.on_chunk_header = HttpContext::onChunkHeader;
  settings_.on_chunk_complete = HttpContext::onChunkComplete;
}

HttpContext::HttpContext(const HttpContext& rhs)
  : parser_(rhs.parser_),
    settings_(rhs.settings_),
    request_(rhs.request_),
    receiveTime_(rhs.receiveTime_),
    lastHeaderCallback_(rhs.lastHeaderCallback_),
    field_(rhs.field_),
    value_(rhs.value_),
    requestComplete_(rhs.requestComplete_)
{
  parser_.data = this;
}

const HttpContext& HttpContext::operator=(const HttpContext& rhs)
{
  parser_ = rhs.parser_;
  parser_.data = this;

  settings_ = rhs.settings_;
  request_ = rhs.request_;
  receiveTime_ = rhs.receiveTime_;
  lastHeaderCallback_ = rhs.lastHeaderCallback_;
  field_ = rhs.field_;
  value_ = rhs.value_;
  requestComplete_ = rhs.requestComplete_;

  return *this;
}

HttpContext::~HttpContext()
{
}

bool HttpContext::parseRequest(muduo::net::Buffer* buffer,
                               muduo::Timestamp receiveTime)
{
  receiveTime_ = receiveTime;

  size_t parsedBytes = http_parser_execute(&parser_, &settings_,
                                           buffer->peek(),
                                           buffer->readableBytes());

  if (parsedBytes != buffer->readableBytes())
  {
    return false;
  }

  buffer->retrieve(parsedBytes);

  return true;
}

int HttpContext::onMessageBegin(http_parser* parser)
{
  HttpContext* context = static_cast<HttpContext*>(parser->data);
  context->onMessageBegin();
  return 0;
}

int HttpContext::onUrl(http_parser* parser, const char* at, size_t length)
{
  HttpContext* context = static_cast<HttpContext*>(parser->data);
  context->onUrl(muduo::StringPiece(at, static_cast<int>(length)));
  return 0;
}

int HttpContext::onStatus(http_parser* parser, const char* at, size_t length)
{
  HttpContext* context = static_cast<HttpContext*>(parser->data);
  context->onStatus(muduo::StringPiece(at, static_cast<int>(length)));
  return 0;
}

int HttpContext::onHeaderField(http_parser* parser, const char* at, size_t length)
{
  HttpContext* context = static_cast<HttpContext*>(parser->data);
  context->onHeaderField(muduo::StringPiece(at, static_cast<int>(length)));
  return 0;
}

int HttpContext::onHeaderValue(http_parser* parser, const char* at, size_t length)
{
  HttpContext* context = static_cast<HttpContext*>(parser->data);
  context->onHeaderValue(muduo::StringPiece(at, static_cast<int>(length)));
  return 0;
}

int HttpContext::onHeadersComplete(http_parser* parser)
{
  HttpContext* context = static_cast<HttpContext*>(parser->data);
  context->onHeadersComplete();
  return 0;
}

int HttpContext::onBody(http_parser* parser, const char* at, size_t length)
{
  HttpContext* context = static_cast<HttpContext*>(parser->data);
  context->onBody(muduo::StringPiece(at, static_cast<int>(length)));
  return 0;
}

int HttpContext::onMessageComplete(http_parser* parser)
{
  HttpContext* context = static_cast<HttpContext*>(parser->data);
  context->onMessageComplete();
  return 0;
}

int HttpContext::onChunkHeader(http_parser* parser)
{
  LOG_INFO << "HttpContext::onChunkComplete";
  return 0;
}

int HttpContext::onChunkComplete(http_parser* parser)
{
  LOG_INFO << "HttpContext::onChunkComplete";
  return 0;
}

void HttpContext::onMessageBegin()
{
  request_.setReceiveTime(receiveTime_);
}

void HttpContext::onUrl(const muduo::StringPiece& url)
{
  LOG_INFO << "HttpContext::onUrl url: " << url;
}

void HttpContext::onStatus(const muduo::StringPiece& status)
{
  LOG_INFO << "HttpContext::onStatus status: " << status;
}

void HttpContext::onHeaderField(const muduo::StringPiece& field)
{
  LOG_INFO << "HttpContext::onHeaderField field: " << field;
  if (lastHeaderCallback_ == kHeaderFieldCallback)
  {
    field_ += field.as_string();
  }
  else
  {
    field_ = field.as_string();
  }
  lastHeaderCallback_ = kHeaderFieldCallback;
}

void HttpContext::onHeaderValue(const muduo::StringPiece& value)
{
  LOG_INFO << "HttpContext::onHeaderValue value: " << value;
  if (lastHeaderCallback_ == kHeaderValueCallback)
  {
    value_ += value.as_string();
  }
  else
  {
    value_ = value.as_string();
  }
  lastHeaderCallback_ = kHeaderValueCallback;
  request_.addHeader(field_, value_);
}

void HttpContext::onHeadersComplete()
{
  LOG_INFO << "HttpContext::onHeadersComplete";
  std::map<muduo::string, muduo::string>::const_iterator citer;
  for (citer = request_.headers().begin();
       citer != request_.headers().end();
       citer++)
  {
    LOG_INFO << citer->first << ": " << citer->second;
  }
}

void HttpContext::onBody(const muduo::StringPiece& body)
{
  LOG_INFO << "HttpContext::onBody body: " << body;
}

void HttpContext::onMessageComplete()
{
  LOG_INFO << "HttpContext::onMessageComplete";
  requestComplete_ = true;
}
