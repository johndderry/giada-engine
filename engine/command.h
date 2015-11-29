class String {
private:
	char *base;
	int  length;

public:

	String();
	~String();
	String(char *s, int l);
	String(const char *s, int l);
	char *gets();
	int  len();
	int  compare(String *);
	int  compare(const char *);
	bool  match(const char *);
	String **split(const char *);
	int  append(char);
	int  append(char *);

};

/****************************************************************/

class Unsigned {
private:
	unsigned val;

public:
	Unsigned(char *);
	unsigned value();

};

/****************************************************************/


class CommandHandler {

public:

	CommandHandler();
	Channel *locate( unsigned val ) ;
	void parseSampleArguments( Unsigned *ch_addr, String **arguments );
	void parseMidiArguments( Unsigned *ch_addr, String **arguments );
	String *parse(char *, int);

};

