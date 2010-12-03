import java.net.*;
import java.io.*;

// For decompressing with ZLIB
import java.util.zip.*;

// Random numbers
import java.util.Random;

/// Esta clase implementa la logica basica de un servidor que funciona con sockets
/// Probablemente esta misma clase haya que ponerla en un Thread y lanzarla desde
/// el servidor principal
public class OniServer
{
  // El puerto que ser abrira para las conexiones entrantes de los clientes Sink
  private static int portNumber = 4444;

  public static void main(String[] args)
    throws IOException
  {
    ServerSocket serverSocket = null;
    boolean listening = true;

    System.out.println("Starting Oni Server... ");

    try {
      serverSocket = new ServerSocket(portNumber);
    }
    catch (IOException e) {
      System.err.println("Could not listen on port: " + portNumber + ".");
      System.exit(1);
    }

    System.out.println("Started... Starting simulation...");
    new NewClientsThread().start();

    System.out.println("Started... Awaiting for connections...");

    while (listening)
      new OniServerThread( serverSocket.accept() ).start();

    serverSocket.close();
  }
}

/// Esta clase deberia estar atento a los clientes que crean nuevas cuentas
/// y decirle a Boinc que hay un nuevo trabajo cada vez que se cree una
/// En vez de eso, sencillamente ejecuta un instancia del cliente Sink ;)
class NewClientsThread
  extends Thread
{
  public void run()
  {
    // Reactions per dot: Numero de iteraciones que deben ocurrir para que el
    // cliente Sink envie la informacion sobre el estado a Oni.
    // 1000 me parece un buen numero pero que habra que ajustarlo despues
    // cuando ajustemos todos los demas parametros.
    int rpd = 1000; // reactions per dot
    try {
      // FIXME This should wait for a client to create an account instead
      Process proc = Runtime.getRuntime().exec("sh sink_client.sh localhost " + rpd);
      proc.waitFor();
    }
    catch (Exception e) {
      System.out.println("Error running simulation: " + e);
      e.printStackTrace();
    }
    return;
  }
}

/// Esta clase es la que esta a cargo de la comunicacion con el cliente Sink.
class OniServerThread
  extends Thread
{
  private Socket socket = null;

  public OniServerThread(Socket socket)
  {
    super("OniServerThread");
    this.socket = socket;
    System.out.println("New connection opened with " + socket.getInetAddress());
  }

  public void run()
  {
    try {
      InputStream is = socket.getInputStream();
      DataInputStream in = new DataInputStream(is);

      PrintWriter out = new PrintWriter(socket.getOutputStream(), false);

      while (true)
      {
        byte[] buf = new byte[bufSize];
        // Recibimos el string comprimido que tiene el estado de la simulacion
        int nbytes = in.read(buf, 0, bufSize);
        if (nbytes == -1)
          break;
        System.out.println("Received: \"" + buf + "\" (" + nbytes + " bytes long)");
        // Descomprime el string enviado desde Sink
        String decoded = decode( buf, nbytes );
        System.out.println("Decoded: \"" + decoded + "\"");
        if (decoded == "")
        { // El 
          System.out.println("Error decoding input packet: Client must die");
          out.write( "Die" );
          out.flush();
        }
        else
        {
          // FIXME This should tell Sink the actual modifications the user has made, not some random stuff
          String output = generateRandomOutputString( decoded );
          out.println( output );
          out.flush();
          System.out.println("Sent: \"" + output + "\"");
        }
      }

      in.close();
      out.close();
      socket.close();
    }
    catch (IOException e) {
      e.printStackTrace();
    }
  }

  /// Este metodo descomprime la cadena de bytes 'input' usando Zlib
  private String decode(byte[] input, int nbytes)
  {
    try {
      Inflater decompresser = new Inflater();
      decompresser.setInput(input, 0, nbytes);
      byte[] result = new byte[bufSize];
      int resultLength = decompresser.inflate(result);
      decompresser.end();
      return new String(result, 0, resultLength, "UTF-8");
    }
    catch(java.io.UnsupportedEncodingException e) {
      System.err.println("Unsupported Encoding Exception: " + e);
      e.printStackTrace();
    }
    catch(DataFormatException e) {
      System.err.println("Data Format Exception: " + e);
      e.printStackTrace();
    }
    return ""; // This is completely unnecessary, but Java requires it
  }

  /// Esta funcion genera una respuesta aleatoria para enviarle de vuelta a Sink.
  /// Ahora que lo integremos debemos cambiar esto para que entregue las modificaciones
  /// que ha hecho el usario.
  private String generateRandomOutputString(String state)
  {
    String output = "";
    Random rng = new Random();
    double p = rng.nextDouble();
    if (p < 0.1)
    { // Add a new object
      output += '+';
      int i = rng.nextInt( objsList.length );
      output += objsList[i];
    }
    else if (p < 0.2)
    { // Delete an object
      output += '-';
      String[] agents = state.split(";");
      System.out.println("num agents = " + agents.length);
      int i = rng.nextInt( agents.length );
      output += agents[i];
    }
    return output;
  }

  private static int bufSize = 8192;
  private static String[] objsList = {"Mit", "RER", "REL", "Cyt", "Nucleo", "ATP", "Prot", "Lip"};
}
