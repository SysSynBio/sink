import java.io.*;
import java.net.*;

// For compressing with ZLIB
import java.util.zip.*;

// Random numbers
import java.util.Random;


public class Client
{
  private static int portNumber = 4444;

  public static void main(String[] args)
    throws IOException, InterruptedException
  {
    Socket socket = null;
    BufferedReader in = null;
    OutputStream os = null;
    DataOutputStream out = null;

    if (args.length != 1)
    {
      System.err.println("Give me a hostname.");
      System.exit(1);
    }

    String hostname = args[0];

    try {
      System.out.println("Connecting to server: " + hostname + " port: " + portNumber);
      socket = new Socket(hostname, portNumber);
      os = socket.getOutputStream();
      out = new DataOutputStream(os);
      in = new BufferedReader(
             new InputStreamReader(
               socket.getInputStream()));
    }
    catch (SocketException e)
    {
      System.err.println("Socket error : " + e);
      System.exit(1);
    }
    catch (UnknownHostException e)
    {
      System.err.println("Don't know about host: " + hostname);
      System.exit(1);
    }
    catch (IOException e)
    {
      System.err.println("Couldn't get I/O for the connection to: " + hostname);
      System.exit(1);
    }

    System.out.println("Connected.");

    String mixture = generateInitialMixture();
    for (int i = 0; i < 3; ++i)
    {
      // Send a sequence
      int nbytes = 0;
      EncodedString encoded = encode( mixture );
      out.write(encoded.contents, 0, encoded.length);
      out.flush();

      System.out.println("Sent: " + encoded.contents + " (" + encoded.length + " bytes long)");

      // Read response
      String inputLine = in.readLine();
      System.out.println("Received: \"" + inputLine + "\"");

      System.out.println("and going to sleep for one second...");
      // Sleep 1 sec
      Thread.sleep(1000);
    }

    out.close();
    in.close();
    socket.close();
  }

  private static EncodedString encode(String inputStr)
  {
    try {
      byte[] input = inputStr.getBytes("UTF-8");
      byte[] output = new byte[bufSize];
      Deflater compresser = new Deflater();
      compresser.setInput(input);
      compresser.finish();
      int nbytes = compresser.deflate(output);
      return new EncodedString(output, nbytes);
    }
    catch(java.io.UnsupportedEncodingException e) {
      System.err.println("Unsupported Encoding Exception: " + e);
      e.printStackTrace();
      System.exit(1);
    }
    return new EncodedString(); // Don't dare to take this line out... Java is too stupid
  }

  private static String generateInitialMixture()
  {
    Random rng = new Random();
    String initialMixture = "Cyt";
    for (int i = 0; i < numAgents; ++i)
    { 
      int j = rng.nextInt( objsList.length );
      initialMixture += ";" + objsList[j];
    }
    return initialMixture;
  }

  private static int numAgents = 10000;
  private static int bufSize = 8192;
  private static String[] objsList = {"Mit", "RER", "REL", "Cyt", "Nucleo", "ATP", "Prot", "Lip"};
}

class EncodedString
{
  public EncodedString() {}

  public EncodedString(byte[] bytes, int len)
  {
    contents = bytes;
    length = len;
  }

  public byte[] contents;
  public int length;
}
