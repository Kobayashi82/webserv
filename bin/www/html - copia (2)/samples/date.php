<?php
date_default_timezone_set('Europe/Madrid');

$currentDate = date('Y-m-d H:i:s');

$htmlContent = "
<h3>The current date and time is: $currentDate</h3>
";

echo $htmlContent;
?>
