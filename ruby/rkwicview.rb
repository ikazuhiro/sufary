#!/usr/local/bin/ruby
# $Id: rkwicview.rb,v 1.2 2000/07/15 07:45:35 kazuma-t Exp $
#
# rkwicview: KeyWord In Context VIEWer for Ruby.
# This program works as a simple HTTP daemon.

require 'socket'
require 'getopts'
require 'kconv'
require 'sufary'

HTTPVERSION = "HTTP/1.0"

#
# very simple HTTP server
#
class SimpleHTTPServer
  def initialize(port = 0, version = "HTTP/1.0")
    @accepter = TCPServer::open(port)
    @version = version
    if port == 0 then
      addr = @accepter.addr
      addr.shift
      $stderr.printf("server is on %d\n", addr.join(':'))
    end
  end

  def methodOptions(socket, uri, version)
    methodUnsupported(socket, uri, version)
  end

  def methodGet(socket, uri, version)
    methodUnsupported(socket, uri, version)
  end

  def methodHead(socket, uri, version)
    methodUnsupported(socket, uri, version)
  end

  def methodPost(socket, uri, version)
    methodUnsupported(socket, uri, version)
  end

  def methodPut(socket, uri, version)
    methodUnsupported(socket, uri, version)
  end

  def methodDelete(socket, uri, version)
    methodUnsupported(socket, uri, version)
  end

  def methodTrace(socket, uri, version)
    methodUnsupported(socket, uri, version)
  end

  def methodUnsupported(socket, uri, version)
    socket.write("#{@version} 501 Not Implemented\n\n")
  end

  def accept
    while true
      socket = @accepter.accept
      if isForbitten(socket) then
	socket.close
	return
      end
      Thread.start do
	s = socket
	line = s.gets
	method, uri, version = line.split(/\s/)
	case method
	when "OPTIONS"
	  methodOptions(s, uri, version)
	when "GET"
	  methodGet(s, uri, version)
	when "HEAD"
	  methodHead(s, uri, version)
	when "POST"
	  methodPost(s, uri, version)
	when "PUT"
	  methodPut(s, uri, version)
	when "DELETE"
	  methodDelete(s, uri, version)
	when "TRACE"
	  methodTrace(s, uri, version)
	else
	  methodUnsupported(s, uri, version)
	end
	s.close
      end
    end
  end

  def isForbitten(socket)
#    hostname = socket.peeraddr[2]
#    ipaddr = socket.peeraddr[3]
    return false
  end
end


#
# HTML
#
def printHTMLHeader(io)
  io.write <<"EOF"
#{HTTPVERSION} 200 Document follows
Content-type: text/html

<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01//EN"
   "http://www.w3.org/TR/html4/strict.dtd">
<HTML lang="ja">
<HEAD>
  <META http-equiv="content-type" content="text/html; charset=euc-jp">
  <META name="description" content="kwickview">
  <TITLE lang="ja">kwicview</TITLE>
  <STYLE type="text/css">
  <!--
    TD.pre { text-align: right; }
    TD.keyword { text-align: center; background: pink; }
    TD.post { text-align: left; }
  -->
  </STYLE>

</HEAD>
<BODY>
EOF
end

def printHTMLFooter(io)
  io.write <<"EOF"
</BODY>
</HTML>
EOF
end

def printQueryBox(io, keyword)
  io.write <<"EOF"
<FORM action="http://#{io.addr[2]}:#{io.addr[1]}#{$filename}" method="post">
<P>
 <LABEL>Keyword: </LABEL>
   <INPUT type="text" name="keyword" value="#{keyword}">
   <INPUT type="submit" value="Search">
   <INPUT type="reset" value="Reset">
</P>
</FORM>
<FORM action="http://#{io.addr[2]}:#{io.addr[1]}/" method="post">
<P>
<INPUT type="submit" name="quit" value="Quit">
</P>
</FORM>
EOF
end

def printKWICTable(io, keyword)
  if keyword == "" then
    return
  end
  ary = Sufary.new($filename)
  result = ary.search(keyword)
  if result.size == 0 then
    io.write("<P>Not Found.</P>\n")
    return
  end

  io.write <<"EOF"
<TABLE border="1"
    summary="KWIC of #{keyword}">
<COLGROUP span="3">
</COLGROUP>

EOF

  result.each {|i|
    pre, key, post = ary.get_context_lines(i)
    post.chomp
    io.write("<TR>\n")
    [['pre', pre], ['keyword', keyword], ['post', post]].each { |v|
      name, value = v
      io.write('<TD class="' + name + '">')
      io.write(value)
      io.write("\n")
    }
  }
  io.write("</TABLE>\n")
end

def quit(io)
  printHTMLHeader(io)
  io.write("<P>Quit kwicview server.</P>\n")
  printHTMLFooter(io)
  io.close
  exit(0)
end

#
# utils
#
def printUsage
  $stderr.puts("Usage: rkwicview [-p NUM] [FILE]")
  $stderr.puts("Try `rkwicview --help' for more information")
end

def printHelp
  $stderr.print <<EOF
Usage: rkwicview [-p NUM] [FILE]
      -p [NUM]  Use NUM port for HTTP.
EOF
end

def decode(string)
  return Kconv::toeuc(string.gsub(/\+/, " ").gsub(/%([0-9a-fA-F]{2})/n){ [$1.hex].pack("c") })
end

#
# KWIC
#
def printPage(socket, keyword = "")
  socket.write("#{HTTPVERSION} 200 OK")
  printHTMLHeader(socket)
  socket.write("<P>File: " + $filename + "</P>\n")
  printQueryBox(socket, keyword)
  printKWICTable(socket, keyword)
  printHTMLFooter(socket)
end

#
# main
#

unless getopts(nil, "p:0")
  $stderr.puts("kwicview: illegal option")
  printUsage()
  exit(2)
end

$filename = ""
unless ARGV.empty? then
  $filename = ARGV.shift
end

server = SimpleHTTPServer.new($OPT_p.to_i, HTTPVERSION)

def server.methodGet(socket, uri, version)
#  $stderr.print ["GET", uri, version].join(" ") + "\n"
  until /^\s*$/ =~ socket.gets
#    $stderr.print
  end
  unless uri == "/" then
    $filename = uri
  end
  printPage(socket)
end
def server.methodPost(socket, uri, version)
#  $stderr.print ["POST", uri, version].join(" ") + "\n"
  until /^\s*$/ =~ socket.gets
#    $stderr.print
    if /^Content-length:/ then
      length = split(/\s/).pop.to_i
    end
  end

  query = socket.read(length)
  if /^quit/ =~ query then
    quit(socket)
  elsif /^keyword/ =~ query then
    printPage(socket, decode(query.split(/=/,2)[1]))
  else
    $stderr.put("can't reach here")
    exit(100)
  end
end

server.accept
