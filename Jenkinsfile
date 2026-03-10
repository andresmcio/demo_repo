pipeline {
    agent any // El nodo debe tener Docker disponible en el host

    parameters {
        string(name: 'SCANOSS_API_TOKEN_ID', defaultValue:"scanoss-token", description: 'scanoss-token')
        string(name: 'SCANOSS_CLI_DOCKER_IMAGE', defaultValue:"ghcr.io/scanoss/scanoss-py-jenkins:v1.46.0", description: 'Docker Image')
        string(name: 'SCANOSS_API_URL', defaultValue:"https://api.osskb.org/scan/direct", description: 'https://api.scanoss.com')
        string(name: 'SCAN_PATH', defaultValue: '.', description: 'Path to scan')
        booleanParam(name: 'DEBUG', defaultValue: false , description: 'Enable debugging')
    }

    environment {
        SCANOSS_RESULTS_JSON = "results.json"
        SCANOSS_SARIF = "results.sarif"
    }

    stages {
        stage('SCANOSS Analysis') {
            agent {
                docker {
                    image params.SCANOSS_CLI_DOCKER_IMAGE
                    // QUITAMOS el -u 1000:1000 y el -u 0:0 manual. 
                    // Dejamos solo la plataforma y que Jenkins maneje el usuario.
                    args '--platform linux/arm64' 
                    reuseNode true
                }
            }
            steps {
               script {
                   echo "--- Probando conexión dentro del contenedor ---"
                   // Verificamos que el binario de scanoss responda
                   sh "scanoss-py --version"

                   // Ejecución del escaneo
                   sh "scanoss-py scan . --apiurl ${params.SCANOSS_API_URL} --output results.json"

                   // Conversión a SARIF
                   sh "scanoss-py convert --input results.json --format sarif --output results.sarif"
                   
                   // Publicar gráficos
                   recordIssues(
                       tools: [sarif(pattern: 'results.sarif', id: 'scanoss', name: 'SCANOSS Analysis')],
                       qualityGates: [[threshold: 1, type: 'TOTAL', criticality: 'UNSTABLE']]
                   )
                }
            }
        }
    }
}

def scan() {
    withCredentials([string(credentialsId: params.SCANOSS_API_TOKEN_ID, variable: 'SCANOSS_API_TOKEN')]) {
        script {
            def cmd = "scanoss-py scan . --apiurl ${params.SCANOSS_API_URL} --output ${env.SCANOSS_RESULTS_JSON}"
            if(env.SCANOSS_API_TOKEN) { cmd += " --key ${SCANOSS_API_TOKEN}" }
            if(params.DEBUG) { cmd += " --debug" }
            
            // returnStatus: true evita que el pipeline muera si el escaneo detecta algo (queremos llegar a los gráficos)
            sh script: cmd, returnStatus: true
        }
    }
}