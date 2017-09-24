#include "common.h"

using asio::ip::tcp;

struct pos
{
	int x;
	int y;
};

typedef std::ostream ostream;
typedef std::istream istream;
typedef std::string string;

struct MoveInfo
{

	friend istream & operator>>(istream &in, MoveInfo &obj);
	friend ostream & operator<<(ostream &out, MoveInfo &obj);

	int pid;
	pos src;
	pos dest;
	string info;

	char body_[512];
	int body_length_;

	MoveInfo():info("中国人")
	{
		pid = 0;
		src.x = 12;
		src.y = 123;
		dest.x = 33;
		dest.y = 90;

	}
	
	void encode()
	{
		body_length_=0;
		put_int(pid);
		put_int(src.x);
		put_int(src.y);
		put_int(dest.x);
		put_int(dest.y);
		put_string(info);
	}

	void put_string(std::string s)
	{
		int len = strlen(s.c_str());
		std::memcpy(body_+body_length_,s.c_str(),len);
		body_length_+=len;
	}

	void put_int(int x)
	{
		char int_holder[5] = "";
		std::sprintf(int_holder,"%4d",x);
		std::memcpy(body_+body_length_,int_holder,sizeof(int));
		body_length_+=sizeof(int);
	}

	void get_int(int & out,int& pos)
	{
		char int_holder[5] = "";
		std::strncat(int_holder,body_+pos,sizeof(int));
		out = std::atoi(int_holder);
		pos+=sizeof(int);
	}

	std::string get_string(int & pos)
	{
		char string_holder[256] = "";
		std::strncat(string_holder,body_ + pos, body_length_-pos );

		return std::string(string_holder);
	}

	void decode()
	{	
		int pos=0;
		get_int(pid,pos);
		get_int(src.x,pos);
		get_int(src.y,pos);
		get_int(dest.x,pos);
		get_int(dest.y,pos);
		info = get_string(pos);
	}

	char* body()
	{	
		return body_;
	}

	int body_length()
	{
		return body_length_;
	}
	void body_length(int len)
	{
		body_length_ = len;
	}

};

ostream & operator<<(ostream &out,MoveInfo &obj)
{
	//MoveInfo &obj = *this;
	out  <<"pid:" << obj.pid << std::endl
	<<"src x:" << obj.src.x << std::endl
	<<"src y:"<< obj.src.y<< std::endl
	<<"dest x:"<< obj.dest.x<< std::endl
	<<"dest y:"<< obj.dest.y<< std::endl
	<<"info:"<< obj.info;
	return out;
}

istream & operator>>(istream &in,MoveInfo& obj)
{
	//MoveInfo &obj = *this;
	in  >> obj.pid 
	>> obj.src.x >> obj.src.y
	>> obj.dest.x >> obj.dest.y
	>> obj.info;
	if (!in)
	{
		obj = MoveInfo();
	}
	return in;
}



class message
{
public:
	enum 
	{
		HEADER_LEN = 4,
		MAX_BODY_LEN = 512
	};

	message() 
	:body_length_(0)
	{

	}

	const char* data() const
	{
		return data_;
	}

	char* data()
	{
		return data_;
	}

	std::size_t length() const
	{
		return HEADER_LEN + body_length_;
	}
	const char* body() const
	{
		return data_ + HEADER_LEN;
	}
	char* body() 
	{
		return data_+ HEADER_LEN;
	}

	std::size_t body_length() const
	{
		return body_length_;
	}

	void body_length(std::size_t new_length)
	{
		body_length_ = new_length;
		if (body_length_ > MAX_BODY_LEN)
			body_length_ = MAX_BODY_LEN;
	}

	bool decode_header()
	{
		char header[HEADER_LEN +  1] = "";
		std::strncat(header,data_,HEADER_LEN);
		body_length_ = std::atoi(header);
		if( body_length_ > MAX_BODY_LEN)
		{
			body_length_ = 0;
			return false;
		}
		return true;
	}

	void encode_header()
	{
		char header[HEADER_LEN + 1] = "";
		std::sprintf(header,"%4d",static_cast<int>(body_length_));
		std::memcpy(data_,header,HEADER_LEN);
	}

private:
	char data_[HEADER_LEN + MAX_BODY_LEN];
	std::size_t body_length_;

};

class session
: public std::enable_shared_from_this<session>
{
public:
	session(tcp::socket socket)
	: socket_(std::move(socket))
	{
	}

	void start()
	{
		do_read();
	}

private:
	void do_read()
	{
		auto self(shared_from_this());
		socket_.async_read_some(asio::buffer(data_, max_length),
			[this, self](std::error_code ec, std::size_t length)
			{
				if (!ec)
				{
					message msg;	
					std::memcpy(msg.data(), data_, length);
					msg.decode_header();

					MoveInfo info;
					std::memcpy(info.body(), msg.body(), msg.body_length());
					info.body_length(msg.body_length());
					info.decode();

					std::cout <<"receie ok : "<< info <<std::endl;

					// std::cout << "receive msg len:" << length <<std::endl;
					// std::cout.write(msg.body(), msg.body_length());
            		// std::cout << "\n";

					do_write(length);
				}
			});
	}

	void do_write(std::size_t length)
	{
		auto self(shared_from_this());
		asio::async_write(socket_, asio::buffer(data_, length),
        [this, self](std::error_code ec, std::size_t /*length*/)
		{
			if (!ec)
			{
				do_read();
			}
		});
	}

	tcp::socket socket_;
	enum { max_length = 1024 };
	char data_[max_length];
};

class server
{
public:
	server(asio::io_context& io_context, short port)
	: acceptor_(io_context, tcp::endpoint(tcp::v4(), port))
	{
		do_accept();
	}

private:
	void do_accept()
	{
		acceptor_.async_accept(
			[this](std::error_code ec, tcp::socket socket)
			{
				if (!ec)
				{
					std::cout <<"new connection!" << std::endl;
					std::make_shared<session>(std::move(socket))->start();
				}

				do_accept();
			});
	}

	tcp::acceptor acceptor_;
};

int main(int argc,char* argv[])
{	
	try
	{
		if (argc != 2)
		{
			std::cerr << "Usage: async_tcp_echo_server <port>\n";
			return 1;
		}

		asio::io_context io_context;

		server s(io_context, std::atoi(argv[1]));

		io_context.run();
	}
	catch (std::exception& e)
	{
		std::cerr << "Exception: " << e.what() << "\n";
	}

	return 0;
}