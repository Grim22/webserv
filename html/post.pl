#!/usr/bin/env perl

=head1 DESCRIPTION

printenv — a CGI program that just prints its environment

=cut

my $query = "";
read( STDIN, $query, $ENV{CONTENT_LENGTH} ) == $ENV{CONTENT_LENGTH}
  or return undef;

# my $length = 128 + $ENV{CONTENT_LENGTH};

print "Content-Type: text/html\n";
# print "Content-Length: $length \n";
print "Connection: keep-alive\n\n";

print "<html>\n";
print "<head><title>POST Result</title><meta charset=\"UTF-8\"></head>\n";
print "<body>\n";
print "<h1>Vous avez posté :</h1>\n";
print "<p>";
print "Content-Type: $ENV{CONTENT_TYPE}";
print "</p>\n";
print "<p>";
print $query;
print "</p>\n";
print "</body>\n";
print "</html>\n";