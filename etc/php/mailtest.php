<%=

mail("rey4@columbia.edu","subject","here is a message from " . $HTTP_SERVER_VARS["HTTP_HOST"])

?

"success"

:

"not one"

%>